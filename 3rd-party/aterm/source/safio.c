#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "safio.h"
#include "byteencoding.h"
#include "memory.h"
#include "encoding.h"

#if __STDC_VERSION__ >= 199901L
  /* "inline" is a keyword */
#else
# ifndef inline
#  define inline /* nothing */
# endif
#endif

#define ISSHAREDFLAG 0x00000080U
#define TYPEMASK 0x0000000fU

#define FUNSHARED 0x00000040U
#define APPLQUOTED 0x00000020U

#define DEFAULTSTACKSIZE 256
#define STACKSIZEINCREMENT 512

#define MINIMUMFREEBUFFERSPACE 10

#define TEMPNAMEPAGESIZE 4096

#define DEFAULTSHAREDTERMARRAYSIZE 1024
#define SHAREDTERMARRAYINCREMENT 2048
#define DEFAULTSHAREDSYMBOLARRAYSIZE 1024
#define SHAREDSYMBOLARRAYINCREMENT 2048

#define INTEGERSTOREBLOCKSINCREMENT 16
#define INTEGERSTOREBLOCKSINCREMENTMASK 0x0000000fU
#define INTEGERSTOREBLOCKSIZE 1024
#define INTEGERSTOREBLOCKSIZEMASK 0x000003ffU
#define INTEGERSTOREBLOCKSIZEBITS 10

#define PROTECTEDMEMORYSTACKBLOCKSINCREMENT 16
#define PROTECTEDMEMORYSTACKBLOCKSINCREMENTMASK 0x0000000fU
#define PROTECTEDMEMORYSTACKBLOCKSIZE 1024


/* COMMON STUFF */

/**
 * Creates a protected memory stack.
 * 
 * The memory this store holds will be returned in the order it was requested (last out, first in).
 * Additionally, all the blocks that are allocated by this store are protected, 
 * so all ATerms that are referenced from inside one of the memory blocks this store holds won't be collected before it is destroyed.
 * Because all memory is pre-allocated, so we only need to increment and decrement a pointer for allocating and freeing memory, 
 * which is WAY faster then calling malloc + free and ATprotect + ATunprotect for every few term pointers we need.
 */
static ProtectedMemoryStack createProtectedMemoryStack(){
	ATerm *block;
	
	ProtectedMemoryStack protectedMemoryStack = (ProtectedMemoryStack) AT_malloc(sizeof(struct _ProtectedMemoryStack));
	if(protectedMemoryStack == NULL) ATerror("Unable to allocate protected memory stack.\n");
	
	block = (ATerm*) AT_alloc_protected(PROTECTEDMEMORYSTACKBLOCKSIZE);
	if(block == NULL) ATerror("Unable to allocate block for the protected memory stack.\n");
	
	protectedMemoryStack->blocks = (ATerm**) AT_malloc(PROTECTEDMEMORYSTACKBLOCKSINCREMENT * sizeof(ATerm*));
	if(protectedMemoryStack->blocks == NULL) ATerror("Unable to allocate blocks for the protected memory stack.\n");
	
	protectedMemoryStack->blocks[0] = block;
	
	protectedMemoryStack->nrOfBlocks = 1;
	
	protectedMemoryStack->currentBlockNr = 0;
	protectedMemoryStack->currentIndex = block;
	protectedMemoryStack->spaceLeft = PROTECTEDMEMORYSTACKBLOCKSIZE;
	
	protectedMemoryStack->freeBlockSpaces = (size_t*) AT_malloc(PROTECTEDMEMORYSTACKBLOCKSINCREMENT * sizeof(size_t));
	if(protectedMemoryStack->freeBlockSpaces == NULL) ATerror("Unable to allocate array for registering free block spaces of the protected memory stack.\n");
	
	return protectedMemoryStack;
}

/**
 * Frees the memory associated with the given protected memory stack.
 * Additionally the protection of the memory blocks that are present in the store will be removed.
 * All references that point to data in this store will be invalid after invoking this method.
 */
static void destroyProtectedMemoryStack(ProtectedMemoryStack protectedMemoryStack){
	size_t i = protectedMemoryStack->nrOfBlocks;
	ATerm **blocks = protectedMemoryStack->blocks;
	do{
		ATerm *block = blocks[--i];
		
		AT_free_protected(block);
	}while(i > 0);
	AT_free(blocks);
	
	AT_free(protectedMemoryStack->freeBlockSpaces);
	
	AT_free(protectedMemoryStack);
}

/**
 * Adds an additional block of memory to the given protected memory stack.
 * A previously allocated block will be reused if possible;
 * otherwise a new block of memory is allocated.
 */
static void expandProtectedMemoryStack(ProtectedMemoryStack protectedMemoryStack){
	ATerm *block;
	
	size_t nrOfBlocks = protectedMemoryStack->nrOfBlocks;
	
	protectedMemoryStack->freeBlockSpaces[protectedMemoryStack->currentBlockNr++] = protectedMemoryStack->spaceLeft;
	
	if(nrOfBlocks == protectedMemoryStack->currentBlockNr){
		block = (ATerm*) AT_alloc_protected(PROTECTEDMEMORYSTACKBLOCKSIZE);
		if(block == NULL) ATerror("Unable to allocate block for the protected memory stack.\n");
		
		if((nrOfBlocks & PROTECTEDMEMORYSTACKBLOCKSINCREMENTMASK) == 0){
			size_t newSize = nrOfBlocks + PROTECTEDMEMORYSTACKBLOCKSINCREMENT;
			protectedMemoryStack->blocks = (ATerm**) AT_realloc(protectedMemoryStack->blocks, newSize * sizeof(ATerm*));
			if(protectedMemoryStack->blocks == NULL) ATerror("Unable to allocate blocks array for the protected memory stack.\n");
			
			protectedMemoryStack->freeBlockSpaces = (size_t*) AT_realloc(protectedMemoryStack->freeBlockSpaces, newSize * sizeof(size_t));
			if(protectedMemoryStack->freeBlockSpaces == NULL) ATerror("Unable to allocate array for registering free block spaces of the protected memory stack.\n");
		}
		
		protectedMemoryStack->blocks[protectedMemoryStack->nrOfBlocks++] = block;
	}else{
		block = protectedMemoryStack->blocks[protectedMemoryStack->currentBlockNr];
	}
	
	protectedMemoryStack->currentIndex = block;
	protectedMemoryStack->spaceLeft = PROTECTEDMEMORYSTACKBLOCKSIZE;
}

/**
 * Returns an array of ATerms from the given protected memory stack.
 * NOTE: If we request a ATerm array that is larger then PROTECTEDMEMORYSTACKBLOCKSIZE, fall back to using malloc.
 */
static ATerm* getProtectedMemoryBlock(ProtectedMemoryStack protectedMemoryStack, size_t size){
	ATerm *memoryBlock;
	
	if(size <= PROTECTEDMEMORYSTACKBLOCKSIZE){
		if(protectedMemoryStack->spaceLeft < size) expandProtectedMemoryStack(protectedMemoryStack);
		
		protectedMemoryStack->spaceLeft -= size;
		memoryBlock = protectedMemoryStack->currentIndex;
		protectedMemoryStack->currentIndex += size;
	}else{
		memoryBlock = (ATerm*) AT_alloc_protected(size);
		if(memoryBlock == NULL) ATerror("Unable to allocated large memoryBlock.\n");
	}
	
	return memoryBlock;
}

/**
 * Releases a certain amount of bytes in the given protected memory stack.
 * NOTE: releasing memory will not 'free' it, but it will make it possible to reuse it.
 * For this reason the protected memory stack will never shrink, but will always remain as large as it was at it's maximum size.
 * This is intended by design and will never cause a problem.
 * NOTE2: If we want to release an ATerm array that is larger then PROTECTEDMEMORYSTACKBLOCKSIZE, it was allocated using malloc and WILL be freed.
 */
static void releaseProtectedMemoryBlock(ProtectedMemoryStack protectedMemoryStack, ATerm *ptr, size_t size){
	if(size <= PROTECTEDMEMORYSTACKBLOCKSIZE){		
		if(protectedMemoryStack->spaceLeft < PROTECTEDMEMORYSTACKBLOCKSIZE){
			protectedMemoryStack->spaceLeft += size;
			protectedMemoryStack->currentIndex -= size;
		}else{
			size_t currentBlockNr = --(protectedMemoryStack->currentBlockNr);
			size_t freeBlockSpace = protectedMemoryStack->freeBlockSpaces[currentBlockNr] + size;
			protectedMemoryStack->spaceLeft = freeBlockSpace;
			protectedMemoryStack->currentIndex = protectedMemoryStack->blocks[currentBlockNr] + PROTECTEDMEMORYSTACKBLOCKSIZE - freeBlockSpace;
		}
	}else{
		AT_free_protected(ptr);
	}
}


/* BYTE BUFFER */

/**
 * Creates a byte buffer, with the given capacity.
 */
ByteBuffer ATcreateByteBuffer(size_t capacity){
	char *buffer;
	
	ByteBuffer byteBuffer = (ByteBuffer) AT_malloc(sizeof(struct _ByteBuffer));
	if(byteBuffer == NULL) ATerror("Failed to allocate byte buffer.\n");
	
	buffer = (char*) AT_malloc(capacity * sizeof(char));
	if(buffer == NULL) ATerror("Failed to allocate buffer string for the byte buffer.\n");
	byteBuffer->buffer = buffer;
	byteBuffer->currentPos = buffer;
	
	byteBuffer->capacity = capacity;
	byteBuffer->limit = capacity;
	
	return byteBuffer;
}

/**
 * Encapsulates the given string in a byte buffer.
 * The position will be set at the start and the limit to the capacity.
 * NOTE: The given string MUST have been allocated using malloc, so it can be passed to free() when destroying this bytebuffer.
 * Alternatively you could set the buffer field to NULL, before passing this buffer to the ATdestroyByteBuffer function and handle the freeing of the buffer manually.
 */
ByteBuffer ATwrapBuffer(char *buffer, size_t capacity){
	ByteBuffer byteBuffer = (ByteBuffer) AT_malloc(sizeof(struct _ByteBuffer));
	if(byteBuffer == NULL) ATerror("Failed to allocate byte buffer.\n");
	
	byteBuffer->buffer = buffer;
	byteBuffer->currentPos = buffer;
	
	byteBuffer->capacity = capacity;
	byteBuffer->limit = capacity;
	
	return byteBuffer;
}

/**
 * Returns the amount of space left in the given byte buffer.
 */
inline size_t ATgetRemainingBufferSpace(ByteBuffer byteBuffer){
	return (size_t) (byteBuffer->limit - (byteBuffer->currentPos - byteBuffer->buffer));
}

/**
 * Flips the byte buffer.
 * Setting the limit to the amount of bytes that are currently present in the buffer and setting the current position to the start of the buffer.
 */
inline void ATflipByteBuffer(ByteBuffer byteBuffer){
	byteBuffer->limit = (size_t) (byteBuffer->currentPos - byteBuffer->buffer);
	byteBuffer->currentPos = byteBuffer->buffer;
}

/**
 * Resets the given byte buffer.
 * The current position will be reset to the start of the buffer and the limit will be set to the capacity.
 * NOTE: Keep in mind that the data in the buffer will not be erased (not set to 0).
 */
inline void ATresetByteBuffer(ByteBuffer byteBuffer){
	byteBuffer->currentPos = byteBuffer->buffer;
	byteBuffer->limit = byteBuffer->capacity;
}

/**
 * Frees the memory associated with the given byte buffer.
 */
void ATdestroyByteBuffer(ByteBuffer byteBuffer){
	AT_free(byteBuffer->buffer);
	
	AT_free(byteBuffer);
}


/* WRITING */

/**
 * Writes the given integer to the byte buffer.
 * The encoding will be done in 'byteenconding.c'.
 */
inline static void writeInt(int value, ByteBuffer byteBuffer){
	byteBuffer->currentPos += BEserializeMultiByteInt(value, byteBuffer->currentPos);
}

/**
 * Writes the given double to the byte buffer.
 * The encoding will be done in 'byteenconding.c'.
 */
/* inline static void writeDouble(double value, ByteBuffer byteBuffer){
	BEserializeDouble(value, byteBuffer->currentPos);
	byteBuffer->currentPos += 8;
} */

/**
 * Returns the number of subTerms the given term has.
 */
inline static size_t getNrOfSubTerms(ATerm term){
	size_t type = ATgetType(term);
	if(type == AT_APPL){
		return ATgetArity(ATgetAFun((ATermAppl) term));
	}else if(type == AT_LIST){
		return ATgetLength((ATermList) term);
	/* }else if(type == AT_PLACEHOLDER){
		return 1; */
	}else{
		return 0;
	}
}

/**
 * Ensures that there is enough space left on the stack of the binary writer after the invocation of this function.
 */
static void ensureWriteStackCapacity(BinaryWriter binaryWriter){
	if((binaryWriter->stackPosition + 1) >= binaryWriter->stackSize){
		binaryWriter->stack = (ATermMapping*) AT_realloc(binaryWriter->stack, (binaryWriter->stackSize += STACKSIZEINCREMENT) * sizeof(struct _ATermMapping));
		if(binaryWriter->stack == NULL) ATerror("The binary writer was unable to enlarge the stack.\n");
	}
}

/**
 * Returns a reference to the next ATerm that needs to be serialized to the stream the binary writer is working on.
 */
static ATerm getNextTerm(BinaryWriter binaryWriter){
	ATerm next = NULL;
	
	/* Make sure the stack remains large enough */
	ensureWriteStackCapacity(binaryWriter);
	
	/* if(binaryWriter->stackPosition >= 0) */
        {
		size_t type;
		ATerm term;
		ATermMapping *child;
		
		ATermMapping *current = &(binaryWriter->stack[binaryWriter->stackPosition]);
		while(current->subTermIndex == current->nrOfSubTerms){
			
			if(binaryWriter->stackPosition-- == 0) return NULL;
			
			current = &(binaryWriter->stack[binaryWriter->stackPosition]);
		}
		
		term = current->term;
		type = ATgetType(term);
		
		child = &(binaryWriter->stack[++(binaryWriter->stackPosition)]);
		
		if(type == AT_APPL)
                {
			next = ATgetArgument((ATermAppl) term, current->subTermIndex++);
		}
                else if(type == AT_LIST)
                {
			ATermList nextList = current->nextPartOfList;
			next = ATgetFirst(nextList);
			current->nextPartOfList = ATgetNext(nextList);
			
			current->subTermIndex++;
		/* }else if(type == AT_PLACEHOLDER){
			next = ATgetPlaceholder(term);
			
			current->subTermIndex++; */
		}
                else
                {
			ATerror("Could not find next term. Someone broke the above code.\n");
		}
		
		child->term = next;
		child->nrOfSubTerms = getNrOfSubTerms(next);
		child->subTermIndex = 0; /* Default value */
	}
	
	return next;
}

/**
 * Constructs the header for the given ATerm.
 */
inline static size_t getHeader(ATerm aTerm){
	size_t header = ATgetType(aTerm);

	return header;
}

/**
 * Serializes the given ATermAppl.
 */
static void visitAppl(BinaryWriter binaryWriter, ATermAppl arg, ByteBuffer byteBuffer)
{
	AFun fun = ATgetAFun(arg);
	
	if(binaryWriter->indexInTerm == 0){
		SymEntry symEntry = at_lookup_table[fun];
		size_t funHash = (size_t)((unsigned long) symEntry);
		
		IDMappings sharedAFuns = binaryWriter->sharedAFuns;
		size_t id = IMgetID(sharedAFuns, symEntry, funHash);
		
		size_t header = getHeader((ATerm) arg);
		
		if(id != ATERM_NON_EXISTING_POSITION){
			header |= FUNSHARED;
			*(byteBuffer->currentPos) = (char) header;
			byteBuffer->currentPos++;
			
			assert(id< (((size_t)1)<<(8*sizeof(int)-1))); /* id must fit in an int */
			writeInt((int)id, byteBuffer);
		}else{
			size_t remaining;
			
			char* name = ATgetName(fun);
			size_t isQuoted = ATisQuoted(fun);
			size_t arity = ATgetArity(fun);
			size_t nameLength = strlen(name);
			
			size_t bytesToWrite = nameLength;
			
			if(isQuoted) header |= APPLQUOTED;
			*(byteBuffer->currentPos) = (char) header;
			byteBuffer->currentPos++;
			assert(arity< (((size_t)1)<<(8*sizeof(int)-1))); /* arity must fit in an int */
			writeInt((int)arity, byteBuffer);
			
			assert(nameLength< (((size_t)1)<<(8*sizeof(int)-1))); /* nameLength must fit in an int */
			writeInt((int)nameLength, byteBuffer);
			
			remaining = ATgetRemainingBufferSpace(byteBuffer);
			if(remaining < bytesToWrite){
				bytesToWrite = remaining;
				binaryWriter->indexInTerm = bytesToWrite;
				binaryWriter->totalBytesInTerm = nameLength;
			}
			
			memcpy(byteBuffer->currentPos, name, bytesToWrite);
			byteBuffer->currentPos += bytesToWrite;
			
			id = binaryWriter->currentSharedAFunKey++;
			
			IMmakeIDMapping(sharedAFuns, symEntry, funHash, id);
		}
	}else{
		char* name = ATgetName(fun);
		size_t length = binaryWriter->totalBytesInTerm;
		
		size_t bytesToWrite = length - binaryWriter->indexInTerm;
		size_t remaining = ATgetRemainingBufferSpace(byteBuffer);
		if(remaining < bytesToWrite) bytesToWrite = remaining;
		
		memcpy(byteBuffer->currentPos, (name + binaryWriter->indexInTerm), bytesToWrite);
		byteBuffer->currentPos += bytesToWrite;
		binaryWriter->indexInTerm += bytesToWrite;
		
		if(binaryWriter->indexInTerm == length) binaryWriter->indexInTerm = 0;
	}
}

/**
 * Serializes the given ATermInt
 */
static void visitInt(ATermInt arg, ByteBuffer byteBuffer){
	*(byteBuffer->currentPos) = (char) getHeader((ATerm) arg);
	byteBuffer->currentPos++;
	
	writeInt(ATgetInt(arg), byteBuffer);
}

/**
 * Serializes the given ATermList.
 */
static void visitList(ATermList arg, ByteBuffer byteBuffer)
{
  const size_t n=ATgetLength(arg);
  *(byteBuffer->currentPos) = (char) getHeader((ATerm) arg);
  byteBuffer->currentPos++;
	
  assert(n< (((size_t)1)<<(8*sizeof(int)-1))); /* n must fit in an int */
  writeInt((int)n, byteBuffer);
}

/**
 * Constructs a binary writer that is responsible for serializing the given ATerm.
 */
BinaryWriter ATcreateBinaryWriter(ATerm term){
	ATermMapping *stack;
	ATermMapping *tm;
	
	BinaryWriter binaryWriter = (BinaryWriter) AT_malloc(sizeof(struct _BinaryWriter));
	if(binaryWriter == NULL) ATerror("Unable to allocate memory for the binary writer.\n");
	
	stack = (ATermMapping*) AT_malloc(DEFAULTSTACKSIZE * sizeof(struct _ATermMapping));
	if(stack == NULL) ATerror("Unable to allocate memory for the binaryWriter's stack.\n");
	binaryWriter->stack = stack;
	binaryWriter->stackSize = DEFAULTSTACKSIZE;
	binaryWriter->stackPosition = 0;
	
	binaryWriter->sharedTerms = IMcreateIDMappings(75);
	binaryWriter->currentSharedTermKey = 0;
	
	binaryWriter->sharedAFuns = IMcreateIDMappings(75);
	binaryWriter->currentSharedAFunKey = 0;
	
	binaryWriter->currentTerm = term;
	binaryWriter->indexInTerm = 0;
	
	tm = &(binaryWriter->stack[0]);
	tm->term = term;
	tm->nrOfSubTerms = getNrOfSubTerms(term);
	tm->subTermIndex = 0; /* Default value */
	
	return binaryWriter;
}

/**
 * Checks if the given binary writer is finished with the serialization process.
 * Returns > 0 is true, 0 otherwise.
 */
int ATisFinishedWriting(BinaryWriter binaryWriter){
	int finished = 0;
	if(binaryWriter->currentTerm == NULL) finished = 1;
	
	return finished;
}

/**
 * Frees the memory associated with the given binary writer.
 */
void ATdestroyBinaryWriter(BinaryWriter binaryWriter){
	AT_free(binaryWriter->stack);
	
	IMdestroyIDMappings(binaryWriter->sharedTerms);
	
	IMdestroyIDMappings(binaryWriter->sharedAFuns);
	
	AT_free(binaryWriter);
}

/**
 * Serializes the next part of the ATerm tree associated with the given binary writers to the byte buffer.
 * This function returns when the byte buffer is full; the buffer will be flipped before returning.
 */
void ATserialize(BinaryWriter binaryWriter, ByteBuffer byteBuffer){
	ATerm currentTerm = binaryWriter->currentTerm;
	
	while(currentTerm != NULL && ATgetRemainingBufferSpace(byteBuffer) >= MINIMUMFREEBUFFERSPACE){
		size_t termHash = (size_t)(currentTerm);
		size_t id = IMgetID(binaryWriter->sharedTerms, currentTerm, termHash);
		if(id != ATERM_NON_EXISTING_POSITION){
			*(byteBuffer->currentPos) = (char) ISSHAREDFLAG;
			byteBuffer->currentPos++;
                        assert(id< (((size_t)1)<<(8*sizeof(int)-1))); /* id must fit in an int */
			writeInt((int)id, byteBuffer);
			
			binaryWriter->stackPosition--; /* Pop the term from the stack, since it's subtree is shared. */
		}else{
			size_t type = ATgetType(currentTerm);
			switch(type){
				case AT_APPL:
					visitAppl(binaryWriter, (ATermAppl) currentTerm, byteBuffer);
					break;
				case AT_INT:
					visitInt((ATermInt) currentTerm, byteBuffer);
					break;
				case AT_LIST:
					visitList((ATermList) currentTerm, byteBuffer);
					binaryWriter->stack[binaryWriter->stackPosition].nextPartOfList = (ATermList) currentTerm; /* <- for ATermList->next optimizaton. */
					break;
				default:
					ATerror("%d is not a valid term type.\n", type);
			}
			
			/* Don't add the term to the shared list until we are completely done with it. */
			if(binaryWriter->indexInTerm == 0){
				id = binaryWriter->currentSharedTermKey++;
				IMmakeIDMapping(binaryWriter->sharedTerms, currentTerm, termHash, id);
			}else break;
		}
		
		currentTerm = getNextTerm(binaryWriter);
	}
	
	binaryWriter->currentTerm = currentTerm;
	
	ATflipByteBuffer(byteBuffer);
}


/* READING */

/**
 * Reads an integer from the given byte buffer.
 * The decoding will be done in 'byteencoding.c'.
 */
inline static int readInt(ByteBuffer byteBuffer){
	size_t count;
	int result = BEdeserializeMultiByteInt(byteBuffer->currentPos, &count);
	byteBuffer->currentPos += count;

	return result;
}

/**
 * Reads a double from the given byte buffer.
 * The decoding will be done in 'byteencoding.c'.
 */
/* inline static double readDouble(ByteBuffer byteBuffer){
	double result = BEdeserializeDouble(byteBuffer->currentPos);
	byteBuffer->currentPos += 8;
	
	return result;
} */

/**
 * Ensures that there is enough space left on the stack of the binary reader after the invocation of this function.
 */
static void ensureReadStackCapacity(BinaryReader binaryReader)
{
	if((binaryReader->stackPosition + 1) >= binaryReader->stackSize)
        {
		binaryReader->stack = (ATermConstruct*) AT_realloc(binaryReader->stack, (binaryReader->stackSize += STACKSIZEINCREMENT) * sizeof(struct _ATermConstruct));
		if(binaryReader->stack == NULL) ATerror("Unable to allocate memory for expanding the binaryReader's stack.\n");
	}
}

/**
 * Ensures that there is enough space left in the shared terms array of the binary reader after the invocation of this function.
 */
static void ensureReadSharedTermCapacity(BinaryReader binaryReader){
	if((binaryReader->sharedTermsIndex + 1) >= binaryReader->sharedTermsSize){
		binaryReader->sharedTerms = (ATerm*) AT_realloc(binaryReader->sharedTerms, (binaryReader->sharedTermsSize += SHAREDTERMARRAYINCREMENT) * sizeof(ATerm));
		if(binaryReader->sharedTerms == NULL) ATerror("Unable to allocate memory for expanding the binaryReader's shared terms array.\n");
	}
}

/**
 * Ensures that there is enough space left in the shared signatures array of the binary reader after teh invocation of this function.
 */
static void ensureReadSharedAFunCapacity(BinaryReader binaryReader){
	if((binaryReader->sharedAFunsIndex + 1) >= binaryReader->sharedAFunsSize){
		binaryReader->sharedAFuns = (SymEntry*) AT_realloc(binaryReader->sharedAFuns, (binaryReader->sharedAFunsSize += SHAREDSYMBOLARRAYINCREMENT) * sizeof(SymEntry));
		if(binaryReader->sharedAFuns == NULL) ATerror("Unable to allocate memory for expanding the binaryReader's shared signatures array.\n");
	}
}

/**
 * Resets the temporary reader data of the given binary reader.
 * This temporary data is used when reading 'chunkified types' (name of a function symbol or BLOB);
 * These types can be deserialized in pieces.
 */
inline static void resetTempReaderData(BinaryReader binaryReader){
		binaryReader->tempType = 0;
		
		/* It doesn't matter if tempBytes is NULL, since free(NULL) does nothing */
		AT_free(binaryReader->tempBytes);
		binaryReader->tempBytes = NULL;
		
		binaryReader->tempBytesSize = 0;
		binaryReader->tempBytesIndex = 0;
		
		binaryReader->tempArity = 0;
		binaryReader->tempIsQuoted = 0;
}

/**
 * Adds the given term to the shared terms list.
 */
inline static void shareTerm(BinaryReader binaryReader, ATermConstruct *ac, ATerm term){
	binaryReader->sharedTerms[ac->termKey] = term;
}

/**
 * Constructs the term associated with the given ATerm construct.
 */
static ATerm buildTerm(BinaryReader binaryReader, ATermConstruct *parent){
	ATerm constructedTerm;
	size_t type = parent->type;
	
	if(type == AT_APPL){
		size_t nrOfSubTerms = parent->nrOfSubTerms;
		ATerm *subTerms = parent->subTerms;
		
		SymEntry symEntry = (SymEntry) parent->tempTerm;
		AFun fun = symEntry->id;
		
		/* Use the appropriate way of constructing the appl, depending on if it has arguments or not. */
		if(nrOfSubTerms > 0){
			constructedTerm = (ATerm) ATmakeApplArray(fun, subTerms);
			
			releaseProtectedMemoryBlock(binaryReader->protectedMemoryStack, subTerms, nrOfSubTerms);
		}else{
			constructedTerm = (ATerm) ATmakeAppl0(fun);
		}
		
	}else  if(type == AT_LIST){
		size_t nrOfSubTerms = parent->nrOfSubTerms;
		ATerm *subTerms = parent->subTerms;
		
		ATermList list = ATmakeList0();
		
		if(nrOfSubTerms > 0){
			size_t i = nrOfSubTerms;
			do{
				list = ATinsert(list, subTerms[--i]);
			}while(i > 0);
			
			releaseProtectedMemoryBlock(binaryReader->protectedMemoryStack, subTerms, nrOfSubTerms);
		}
		
		constructedTerm = (ATerm) list;
		
	}else {
		constructedTerm = NULL; /* This line is purely for shutting up the compiler. */
		ATerror("Unable to construct term.\n");
	}
	
	return constructedTerm;
}

/**
 * Links the given (deserialized) term with it's parent.
 */
static void linkTerm(BinaryReader binaryReader, ATerm aTerm){
	ATerm term = aTerm;
	
	while(binaryReader->stackPosition != 0){
		ATermConstruct *parent = &(binaryReader->stack[--(binaryReader->stackPosition)]);
		
		size_t nrOfSubTerms = parent->nrOfSubTerms;
		if(nrOfSubTerms > parent->subTermIndex){
			parent->subTerms[parent->subTermIndex++] = term;
			
			if(nrOfSubTerms != parent->subTermIndex ) return; /* This is the 'normal' return point of this function. */
		}else{
			ATerror("Encountered a term that didn't fit anywhere. Type: %d.\n", ATgetType(term));
		}
		
		term = buildTerm(binaryReader, parent);
	
		shareTerm(binaryReader, parent, term);
	}
	
	if(binaryReader->stackPosition == 0){
		/* Protect the root of the tree, once we're done de-serializing. */
		ATerm *term_ptr = getProtectedMemoryBlock(binaryReader->protectedMemoryStack, 1);
		*term_ptr = term;
		
		binaryReader->isDone = 1;
	}
}

/**
 * Reads bytes from the byte buffer until we either run out of data or the name of the function symbol or BLOB we are reconstructing is complete.
 */
static void readData(BinaryReader binaryReader, ByteBuffer byteBuffer){
	size_t length = binaryReader->tempBytesSize;
	size_t bytesToRead = (length - binaryReader->tempBytesIndex);
	size_t remaining = ATgetRemainingBufferSpace(byteBuffer);
	if(remaining < bytesToRead) bytesToRead = remaining;
	
	memcpy(binaryReader->tempBytes + binaryReader->tempBytesIndex, byteBuffer->currentPos, bytesToRead);
	byteBuffer->currentPos += bytesToRead;
	binaryReader->tempBytesIndex += bytesToRead;
	
	if(binaryReader->tempBytesIndex == length){
		if(binaryReader->tempType == AT_APPL){
			ATermConstruct *ac = &(binaryReader->stack[binaryReader->stackPosition]);
			
			size_t arity = binaryReader->tempArity;
			ATbool isQuoted = binaryReader->tempIsQuoted;
			char *name = binaryReader->tempBytes;
			
			AFun fun = ATmakeAFun(name, arity, isQuoted);
			SymEntry symEntry = at_lookup_table[fun];
			ATprotectAFun(fun);
			
			ensureReadSharedAFunCapacity(binaryReader); /* Make sure we have enough space in the array */
			binaryReader->sharedAFuns[binaryReader->sharedAFunsIndex++] = symEntry;
			
			if(arity > 0){
				ac->tempTerm = (ATerm) symEntry;
				
				ac->subTerms = getProtectedMemoryBlock(binaryReader->protectedMemoryStack, arity);
			}else{
				ATerm term = (ATerm) ATmakeAppl0(fun);
				
				shareTerm(binaryReader, ac, term);
				
				linkTerm(binaryReader, term);
			}
			
			if(length < TEMPNAMEPAGESIZE) 
                             binaryReader->tempBytes = NULL; /* Set to NULL, so we don't free the tempNamePage. */
		}else{
			ATerror("Unsupported chunkified type: %s.\n", binaryReader->tempType);
		}
		
		resetTempReaderData(binaryReader);
	}
}

/**
 * Starts the deserialization of an ATermAppl
 */
static void touchAppl(BinaryReader binaryReader, ByteBuffer byteBuffer, size_t header){
	if((header & FUNSHARED) == FUNSHARED){
		size_t key = readInt(byteBuffer);
		
		SymEntry symEntry = binaryReader->sharedAFuns[key];
		ATermConstruct *ac = &(binaryReader->stack[binaryReader->stackPosition]);
		
		AFun fun = symEntry->id;
		size_t arity = ATgetArity(fun);
		
		if(arity > 0){
			ac->tempTerm = (ATerm) symEntry;
			
			ac->nrOfSubTerms = arity;
			ac->subTerms = getProtectedMemoryBlock(binaryReader->protectedMemoryStack, arity);
		}else{
			ATerm term = (ATerm) ATmakeAppl0(fun);
			
			shareTerm(binaryReader, ac, term);
			
			linkTerm(binaryReader, term);
		}
	}else{
		/* Read arity */
		size_t arity = readInt(byteBuffer);
		
		/* Read name length */
		size_t nameLength = readInt(byteBuffer);
		
		ATermConstruct *ac = &(binaryReader->stack[binaryReader->stackPosition]);
		ac->nrOfSubTerms = arity;
		
		binaryReader->tempArity = arity;
		binaryReader->tempIsQuoted = ((header & APPLQUOTED) == APPLQUOTED);
		
		binaryReader->tempBytesSize = nameLength;
		/* Only allocate a new block of memory if we're dealing with a very large name. */
		if(nameLength < TEMPNAMEPAGESIZE){
			binaryReader->tempBytes = binaryReader->tempNamePage;
		}else{
			binaryReader->tempBytes = (char*) AT_malloc((nameLength + 1) * sizeof(char));
			if(binaryReader->tempBytes == NULL) ATerror("The binary reader was unable to allocate memory for temporary function symbol data.\n");
		}
		binaryReader->tempBytes[nameLength] = '\0'; /* CStrings are \0 terminated. */
		binaryReader->tempBytesIndex = 0;
		binaryReader->tempType = AT_APPL;
		
		/* Read name */
		readData(binaryReader, byteBuffer);
	}
}

/**
 * Starts the deserialization of a ATermList.
 */
static void touchList(BinaryReader binaryReader, ByteBuffer byteBuffer){
	size_t size = readInt(byteBuffer);
	
	ATermConstruct *ac = &(binaryReader->stack[binaryReader->stackPosition]);
	
	if(size > 0){
		ac->nrOfSubTerms = size;
		ac->subTerms = getProtectedMemoryBlock(binaryReader->protectedMemoryStack, size);
	}else {
		ATerm term = (ATerm) ATmakeList0();
		
		shareTerm(binaryReader, ac, term);
		
		linkTerm(binaryReader, term);
	}
}

/**
 * Starts the deserialization of an ATermInt.
 */
static void touchInt(BinaryReader binaryReader, ByteBuffer byteBuffer){
	int value = readInt(byteBuffer);
	ATerm term = (ATerm) ATmakeInt(value);
	
	ATermConstruct *ac = &(binaryReader->stack[binaryReader->stackPosition]);
	
		shareTerm(binaryReader, ac, term);
		
		linkTerm(binaryReader, term);
}

/**
 * Continues the deserialization process, who's state is described in the binary reader with the data in the byte buffer.
 */
void ATdeserialize(BinaryReader binaryReader, ByteBuffer byteBuffer){
	char *endOfBuffer;
	
	if(binaryReader->tempType != 0) readData(binaryReader, byteBuffer);
	
	endOfBuffer = byteBuffer->buffer + byteBuffer->limit; /* Cache the end of buffer pointer, so we don't have to recalculate it every iteration. */
	while(byteBuffer->currentPos < endOfBuffer){
		size_t header = (unsigned char) *(byteBuffer->currentPos);
		byteBuffer->currentPos++;
		
		if((header & ISSHAREDFLAG) == ISSHAREDFLAG){
			size_t termKey = readInt(byteBuffer);
			
			ATerm term = binaryReader->sharedTerms[termKey];
			
			binaryReader->stackPosition++;
			
			linkTerm(binaryReader, term);
		}else{
			size_t type = (header & TYPEMASK);
			
			ATermConstruct *ac = &(binaryReader->stack[++(binaryReader->stackPosition)]);
			
			ensureReadSharedTermCapacity(binaryReader); /* Make sure the shared terms array remains large enough. */
			
			ac->termKey = binaryReader->sharedTermsIndex++;
			
			ac->type = type;
			
			ac->nrOfSubTerms = 0; /* Default value */
			ac->subTermIndex = 0; /* Default value */
			
			switch(type){
				case AT_APPL:
					touchAppl(binaryReader, byteBuffer, header);
					break;
				case AT_LIST:
					touchList(binaryReader, byteBuffer);
					break;
				case AT_INT:
					touchInt(binaryReader, byteBuffer);
					break;
				default:
					ATerror("Unknown type id: %d. Current buffer position: %d\n.", type, (byteBuffer->currentPos - byteBuffer->buffer));
			}
		}
		
		/* Make sure the stack remains large enough. */
		ensureReadStackCapacity(binaryReader);
	}
}

/**
 * Constructs a binary reader that can interpret a SAF stream.
 */
BinaryReader ATcreateBinaryReader(){
	ATermConstruct *stack;
	ATerm *sharedTerms;
	SymEntry *sharedAFuns;
	
	BinaryReader binaryReader = (BinaryReader) AT_malloc(sizeof(struct _BinaryReader));
	if(binaryReader == NULL) ATerror("Unable to allocate memory for the binary reader.\n");
	
	binaryReader->protectedMemoryStack = createProtectedMemoryStack();
	
	stack = (ATermConstruct*) AT_malloc(DEFAULTSTACKSIZE * sizeof(struct _ATermConstruct));
	if(stack == NULL) ATerror("Unable to allocate memory for the binaryReader's stack.\n");
	binaryReader->stack = stack;
	binaryReader->stackSize = DEFAULTSTACKSIZE;
	binaryReader->stackPosition = -1;
	
	sharedTerms = (ATerm*) AT_malloc(DEFAULTSHAREDTERMARRAYSIZE * sizeof(ATerm));
	if(sharedTerms == NULL) ATerror("Unable to allocate memory for the binaryReader's shared terms array.\n");
	binaryReader->sharedTerms = sharedTerms;
	binaryReader->sharedTermsSize = DEFAULTSHAREDTERMARRAYSIZE;
	binaryReader->sharedTermsIndex = 0;
	
	sharedAFuns = (SymEntry*) AT_malloc(DEFAULTSHAREDSYMBOLARRAYSIZE * sizeof(SymEntry));
	if(sharedAFuns == NULL) ATerror("Unable to allocate memory for the binaryReader's shared symbols array.\n");
	binaryReader->sharedAFuns = sharedAFuns;
	binaryReader->sharedAFunsSize = DEFAULTSHAREDSYMBOLARRAYSIZE;
	binaryReader->sharedAFunsIndex = 0;
	
	binaryReader->tempNamePage = (char*) AT_malloc(TEMPNAMEPAGESIZE * sizeof(char));
	if(binaryReader->tempNamePage == NULL) ATerror("Unable to allocate temporary name page.\n");
	
	binaryReader->tempType = 0;
	binaryReader->tempBytes = NULL;
	binaryReader->tempBytesSize = 0;
	binaryReader->tempBytesIndex = 0;
	binaryReader->tempArity = 0;
	binaryReader->tempIsQuoted = ATfalse;
	
	binaryReader->isDone = 0;
	
	return binaryReader;
}

/**
 * Checks if the binary reader is done with the deserialization process.
 * Returns > 0 if true, 0 otherwise.
 */
int ATisFinishedReading(BinaryReader binaryReader){
	return binaryReader->isDone;
}

/**
 * Returns the root of the deserialized tree, present in the given binary writer.
 * NOTE: If the deserialization process is incomplete a warning will be issued and NULL returned.
 */
ATerm ATgetRoot(BinaryReader binaryReader){
	if(binaryReader->isDone <= 0){
		ATwarning("Can't retrieve the root of the tree while it's still being constructed. Returning NULL.");
		return NULL;
	}
	
	return binaryReader->sharedTerms[0]; /* Return the value of the first element (the root of the tree). */
}

/**
 * Frees the memory associated with the given binary reader.
 * NOTE: Calling this function with a binary reader that has started, but not completed, a deserialization process as argument has undefined behavior (protected zones will not be unprotected or freed).
 */
void ATdestroyBinaryReader(BinaryReader binaryReader){
	SymEntry *sharedAFuns = binaryReader->sharedAFuns;
	ptrdiff_t sharedAFunsIndex = binaryReader->sharedAFunsIndex;
	
	destroyProtectedMemoryStack(binaryReader->protectedMemoryStack);
	
	/* We can just free the shared terms, shared signatures and the stack, since they're all present in the memory block store. */
	AT_free(binaryReader->sharedTerms);
	
	AT_free(binaryReader->stack);
	
	while(--sharedAFunsIndex >= 0){
		ATunprotectAFun(sharedAFuns[sharedAFunsIndex]->id);
	}
	AT_free(binaryReader->sharedAFuns);
	
	AT_free(binaryReader->tempNamePage);
	
	resetTempReaderData(binaryReader);
	
	AT_free(binaryReader);
}


/* FILE I/O */

/**
 * Writes the given ATerm in SAF format to the given file.
 * NOTE: The given file must be opened in binary mode (at least on Win32 this is required).
 */
ATbool ATwriteToSAFFile(ATerm aTerm, FILE *file){
	BinaryWriter binaryWriter;
	ByteBuffer byteBuffer;
	
	size_t bytesWritten = fwrite("?", sizeof(char), 1, file);
	if(bytesWritten != 1){
		ATwarning("Unable to write SAF identifier token to file.\n");
		return ATfalse;
	}
	
	binaryWriter = ATcreateBinaryWriter(aTerm);
	byteBuffer = ATcreateByteBuffer(65536);
	
	do{
		size_t blockSize;
		char sizeBytes[2];
		
		ATresetByteBuffer(byteBuffer);
		ATserialize(binaryWriter, byteBuffer);
		
		blockSize = byteBuffer->limit;
		sizeBytes[0] = blockSize & 0x000000ffU;
		sizeBytes[1] = (blockSize >> 8) & 0x000000ffU;
		
		bytesWritten = fwrite(sizeBytes, sizeof(char), 2, file);
		if(bytesWritten != 2){
			ATwarning("Unable to write block size bytes to file.\n");
			ATdestroyByteBuffer(byteBuffer);
			ATdestroyBinaryWriter(binaryWriter);
			return ATfalse;
		}
		
		bytesWritten = fwrite(byteBuffer->buffer, sizeof(char), byteBuffer->limit, file);
		if(bytesWritten != byteBuffer->limit){
			ATwarning("Unable to write bytes to file.\n");
			ATdestroyByteBuffer(byteBuffer);
			ATdestroyBinaryWriter(binaryWriter);
			return ATfalse;
		}
	}while(!ATisFinishedWriting(binaryWriter));
	
	ATdestroyByteBuffer(byteBuffer);
	ATdestroyBinaryWriter(binaryWriter);
	
	if(fflush(file) != 0){
		ATwarning("Unable to flush file stream.\n");
		return ATfalse;
	}
	
	return ATtrue;
}

/**
 * Writes the given ATerm in SAF format to the file with the given name.
 */
ATbool ATwriteToNamedSAFFile(ATerm aTerm, const char *filename){
	ATbool result;
	FILE *file;
	
	if(strcmp(filename, "-") == 0){
		return ATwriteToSAFFile(aTerm, stdout);
	}

	file = fopen(filename, "wb");
	if(file == NULL){
		ATwarning("Unable to open file for writing: %s\n", filename);
		return ATfalse;
	}
	
	result = ATwriteToSAFFile(aTerm, file);
	
	if(fclose(file) != 0) return ATfalse;
	
	return result;
}
 
/**
 * Interprets the content of the given SAF file and returns the constructed ATerm.
 * NOTE: The given file must be opened in binary mode (at least on Win32 this is required)
 */
ATerm ATreadFromSAFFile(FILE *file){
	ATerm term;
	BinaryReader binaryReader;
	ByteBuffer byteBuffer;
	
	char buffer[1];
	size_t bytesRead = fread(buffer, sizeof(char), 1, file); /* Consume the first character in the stream. */
	if(bytesRead <= 0){
		ATwarning("Unable to read SAF id token from file.\n");
		return NULL;
	}
	
	if(buffer[0] != SAF_IDENTIFICATION_TOKEN) ATerror("Not a SAF file.\n");
	
	binaryReader = ATcreateBinaryReader();
	byteBuffer = ATcreateByteBuffer(65536);
	
	do{
		size_t blockSize;
		char sizeBytes[2];
		
		bytesRead = fread(sizeBytes, sizeof(char), 2, file);
		if(bytesRead <= 0) break;
		else if(bytesRead != 2){
			ATwarning("Unable to read block size bytes from file: %d.\n", bytesRead);
			ATdestroyByteBuffer(byteBuffer);
			ATdestroyBinaryReader(binaryReader);
			return NULL;
		}
		blockSize = ((unsigned char) sizeBytes[0]) + (((unsigned char) sizeBytes[1]) << 8);
		if(blockSize == 0) blockSize = 65536;
		
		ATresetByteBuffer(byteBuffer);
		byteBuffer->limit = blockSize;
		bytesRead = fread(byteBuffer->buffer, sizeof(char), blockSize, file);
		if(bytesRead != blockSize){
			ATwarning("Unable to read bytes from file.\n");
			ATdestroyByteBuffer(byteBuffer);
			ATdestroyBinaryReader(binaryReader);
			return NULL;
		}
		
		ATdeserialize(binaryReader, byteBuffer);
	}while(bytesRead > 0);
	
	ATdestroyByteBuffer(byteBuffer);
	
	if(!ATisFinishedReading(binaryReader)){
		ATwarning("Term incomplete, missing data.\n");
		term = NULL;
	}else{
		term = ATgetRoot(binaryReader);
	}
	
	ATdestroyBinaryReader(binaryReader);
	
	return term;
}

/**
 * Interprets the content of the SAF file with the given name and returns the constructed ATerm.
 */
ATerm ATreadFromNamedSAFFile(const char *filename){
	ATerm result;
	FILE *file;
	
	if(strcmp(filename, "-") == 0){
                return ATreadFromSAFFile(stdin);
        }
	
	file = fopen(filename, "rb");
	if(file == NULL){
		ATwarning("Unable to open file for reading: %s\n", filename);
		return NULL;
	}
	
	result = ATreadFromSAFFile(file);
	
	if(fclose(file) != 0) return NULL;
	
	return result;
}

/**
 * This is a node we need to build a linked list of ByteBuffers while writing SAF to a string.
 * By using this strategy, we avoid the unnecessary reallocation of a temporary buffer.
 */
typedef struct _BufferNode{
	ByteBuffer byteBuffer;
	struct _BufferNode *next;
} BufferNode;

/**
 * Writes the given ATerm to a string in SAF format.
 * Since the string will contain \0 bytes, the value the length parameter has after this function returns will specify the number of bytes that were written.
 * Note that the resulting string has been malloced and will need to be freed by the user.
 */
char* ATwriteToSAFString(ATerm aTerm, size_t *length){
	char *result;
	size_t totalBytesWritten = 0;
	size_t position = 0;
	
	BinaryWriter binaryWriter = ATcreateBinaryWriter(aTerm);
	
	BufferNode *last;
	BufferNode *currentBufferNode;
	BufferNode *root = (BufferNode*) AT_malloc(sizeof(struct _BufferNode));
	if(root == NULL) ATerror("Unable to allocate space for BufferNode.\n");
	last = root;
	
	do{
		BufferNode *current;
		ByteBuffer byteBuffer = ATcreateByteBuffer(65536);
		
		ATresetByteBuffer(byteBuffer);
		ATserialize(binaryWriter, byteBuffer);
		
		current = (BufferNode*) AT_malloc(sizeof(struct _BufferNode));
		current->byteBuffer = byteBuffer;
		current->next = NULL;
		last->next = current;
		last = current;
		
		totalBytesWritten += byteBuffer->limit + 2; /* Bytes written per block = buffer size + 2 bytes block size spec. */
	}while(!ATisFinishedWriting(binaryWriter));
	
	ATdestroyBinaryWriter(binaryWriter);
	
	result = (char*) malloc(totalBytesWritten * sizeof(char));
	if(result == NULL) ATerror("Unable to allocate space for result string.\n");
	
	currentBufferNode = root->next;
	AT_free(root);
	do{
		BufferNode *nextBufferNode;
		ByteBuffer currentByteBuffer = currentBufferNode->byteBuffer;
		size_t blockSize = currentByteBuffer->limit;
		
		result[position++] = blockSize & 0x000000ffU;
		result[position++] = (blockSize >> 8) & 0x000000ffU;
		memcpy(result + position, currentByteBuffer->buffer, blockSize * sizeof(char));
		position += blockSize;
		
		ATdestroyByteBuffer(currentByteBuffer);
		
		nextBufferNode = currentBufferNode->next;
		AT_free(currentBufferNode);
		currentBufferNode = nextBufferNode;
	}while(currentBufferNode != NULL);
	
	*length = totalBytesWritten;
	return result;
}

/**
 * Interprets the given string in SAF format and returns the constructed ATerm.
 */
ATerm ATreadFromSAFString(char *data, size_t length)
{
	ATerm term;
	
	size_t position = 0;
	
	BinaryReader binaryReader = ATcreateBinaryReader();
	
	do{
		ByteBuffer byteBuffer;
		int blockSize = (unsigned char) data[position++];
		blockSize += ((unsigned char) data[position++]) << 8;
		if(blockSize == 0) blockSize = 65536;
		byteBuffer = ATwrapBuffer(data + position, blockSize); /* Move the window to the next block. */
		
		ATdeserialize(binaryReader, byteBuffer);
		
		byteBuffer->buffer = NULL; /* Prevent the data string from being freed. */
		ATdestroyByteBuffer(byteBuffer);
		
		position += blockSize;
	}while(position < length);
	
	if(!ATisFinishedReading(binaryReader)){
		ATwarning("Term incomplete, missing data.\n");
		term = NULL;
	}else{
		term = ATgetRoot(binaryReader);
	}
	
	ATdestroyBinaryReader(binaryReader);
	
	return term;
}
