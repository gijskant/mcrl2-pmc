/* $Id: pnml2gs.c,v 1.0 2005/03/09 15:46:00 ysu Exp $ */
#ifdef __cplusplus
extern "C" {
#endif

#define NAME "pnml2gs"
  
#define global_PlaceChars 6          // defines the number of characters added to a place (in actions and processes)
#define global_TransChars 6          // defines the number of characters added to a transition (in actions and processes)
#define global_ArcChars 2            // defines the number of characters added to an arc (in actions and processes)
  
  /* global_MaxIDlength should be at least (1 + max(global_PlaceChars, global_TransChars, global_ArcChars)) */
#define global_MaxIDlength 255       // defines the maximum length of an ID! 
  
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <assert.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <aterm2.h>
#include "libgsparse.h"
#include "gslowlevel.h"
#include "gsfunc.h"
#include "libgsparse.h"
  
  typedef struct {
    ATbool Abort;                 // if an element has no ID, this boolean is used to grant abortion of the conversion
    // read-in
    ATermTable place_name;	// place_id -> name
    ATermTable place_mark;	// place_id -> Nat
    ATermTable trans_name;	// trans_id -> name
    ATermTable arc_in;	        // arc_id -> trans_id x place_id
    ATermTable arc_out;	        // arc_id -> place_id x trans_id
    // generate
    ATermTable place_in;	        // place_id -> List(arc_id) (arc_in)
    ATermTable trans_in;	        // trans_id -> List(arc_id) (arc_out)
    ATermTable place_out;	        // place_id -> List(arc_id) (arc_out)
    ATermTable trans_out;	        // trans_id -> List(arc_id) (arc_in)
    // needed for the creation of general GenSpect processes
    ATermList transitions;      // store all the GenSpect processes (involving transitions) needed for the Trans-process
    ATermList places;           // store all the GenSpect processes (involving places) needed for the PetriNet-process
  } Context;
  static Context context;

  //====================================
  //ADDED BY YARICK: AFun extensions.
  //====================================

  //==================================================
  // ATmakeAFunInt functions as ATmakeAFun, except that the name is an int and not a (char *)
  //==================================================
  static AFun ATmakeAFunInt(int name, int arity, ATbool quoted) {
    // input: an integer value (name), it's arity and whether it is quoted or not
    // output: an AFun, as in ATmakeAFun, but now with a name from an integer value

    // on 128 bit architecture int cannot ocupy more than 128/3+1=43 8-ary digits, even less 10-ary
    char buf[50];
    sprintf(buf, "%d", name);
    return ATmakeAFun(buf, arity, quoted);
  }

  //==================================================
  // ATmakeAFunInt0 functions as ATmakeAFunInt(name,0,ATtrue)
  //==================================================
  static inline AFun ATmakeAFunInt0(int name){
    return ATmakeAFunInt(name, 0, ATtrue);
  }

  //==================================================
  // ATmakeAFunId functions as ATmakeAFun(name,0,ATtrue)
  //==================================================
  static inline AFun ATmakeAFunId(char *name){
    return ATmakeAFun(name, 0, ATtrue);
  }

  //==================================================
  // ATprependAFun functions as ATmakeAFun 
  //==================================================
  static AFun ATprependAFun(const char *str, AFun id) {
    // input: an AFun
    // output: an AFun prepended with str

    char *name=ATgetName(id);
    char *buf=malloc(strlen(str)+strlen(name)+1);
    assert(buf);
    
    strcpy(buf,str);
    strcat(buf,name);

    AFun Res=ATmakeAFun(buf, ATgetArity(id), ATisQuoted(id));
    free(buf);

    return Res;
  }

  //==================================================
  // ATappendAFun functions as ATmakeAFun
  //==================================================
  static AFun ATappendAFun(AFun id, const char *str) {
    // input: an AFun
    // output: an AFun appended with str

    char *name=ATgetName(id);
    char *buf=malloc(strlen(str)+strlen(name)+1);
    assert(buf);
    
    strcpy(buf,name);
    strcat(buf,str);

    AFun Res=ATmakeAFun(buf, ATgetArity(id), ATisQuoted(id));
    free(buf);

    return Res;
  }

  //====================================
  //End ADDED BY YARICK: AFun extensions.
  //====================================


  /*                                 */
  /*                                 */
  /*                                 */
  /* Functions for the read-in phase */
  /*                                 */
  /*                                 */
  /*                                 */

  //==================================================
  // pn2gsCheckString checks if a string is of the format [a-zA-Z_][a-zA-Z0-9_]*
  //==================================================
  static AFun pn2gsCheckAFun(AFun id) {
    // input: an AFun
    // output: an AFun with the string of the format [a-zA-Z_][a-zA-Z0-9_]*
    //         If the input-string is already of this format, it is returned unchanged.
    //         If a string is of a different format, all characters that do not follow the format are replaced with an _
    
    char *String=ATgetName(id);

    // check if the first character is of format [a-zA-Z_]
    if(!(isalpha(String[0])||String[0]=='_')) {
      // first character does not follow the format
      // put 'c_' in front of the String
      id=ATprependAFun("c_",id);
    }

    String=strdup(ATgetName(id));

    for(int i=0; i<strlen(String); i++){
      if(!(isalnum(String[i]))) {
	// if a character in the string is not [a-zA-Z0-9_], replace it by an '_'
	String[i]='_';
      }
    }

    id=ATmakeAFun(String,ATgetArity(id),ATisQuoted(id));
    free(String);
    return id;
  }

  //==================================================
  // pn2gsRetrieveTextWithCheck gets the contents of a child <text> element of cur, and checks it to format [a-zA-Z_][a-zA-Z0-9_]*
  //==================================================
  static ATermAppl pn2gsRetrieveTextWithCheck(xmlNodePtr cur) {
    // input: a pointer to the current element
    // output: the contents of the first child <text> attribute 
    //         of the current element, following format [a-zA-Z_][a-zA-Z0-9_]*
    
    cur=cur->xmlChildrenNode;
    while (cur != NULL) {
      if (!xmlNodeIsText(cur)) {
	if (!xmlStrcmp(cur->name, (const xmlChar *)"text")) {
	  return ATmakeAppl0(pn2gsCheckAFun(ATmakeAFunId((char *)xmlNodeGetContent(cur))));
	  //	  return (ATerm)ATmakeAppl0(ATmakeAFun(RV,0,ATfalse));
	}
      }
      cur=cur->next;
    }
    return NULL;
  }

  //==================================================
  // pn2gsRetrieveText gets the contents of a child <text> element of cur
  //==================================================
  static ATerm pn2gsRetrieveText(xmlNodePtr cur) {
    // input: a pointer to the current element
    // output: the contents of the first child <text> attribute 
    //         of the current element
    
    char * RV;
    cur=cur->xmlChildrenNode;
    while (cur != NULL) {
      if (!xmlNodeIsText(cur)) {
	if (!xmlStrcmp(cur->name, (const xmlChar *)"text")) {
	  RV = (char *)xmlNodeGetContent(cur);
	  return ATparse(RV);
	}
      }
      cur=cur->next;
    }
    return NULL;
  }
  
  //==================================================
  // pn2gsAterm_place converts a pnml-place to a usable ATerm
  //==================================================
  static ATermAppl pn2gsAterm_place(xmlNodePtr cur) {
    // input: a pointer to the current place
    // output: a usable translation of the place,
    //         if the place needs to be translated
    
    gsDebugMsg("> Start examining a place...  \n");
    // first, we want to retrieve the id of the place
    ATermAppl Aid;
    if (!xmlGetProp(cur, (const xmlChar *)"id")) {
      // the place has NO id, so translation should be aborted!
      context.Abort = ATtrue;
      gsErrorMsg("A place has no ID. \n");
      return NULL;
    } else {
      // the place has an id, put it in Aid
      Aid = ATmakeAppl0((pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"id")))));
      
      if (strlen((const char *)xmlGetProp(cur, (const xmlChar *)"id")) > (global_MaxIDlength - global_PlaceChars)) {
	// check if the ID is not too long
	context.Abort = ATtrue;
	gsErrorMsg("The id: '%t' is too long! \n", Aid);
	return NULL;
      }
    }
    gsDebugMsg("    id: '%t'\n", Aid); 
    
    // second, we want to retrieve the necessary attributes of the place
    cur = cur->xmlChildrenNode;
    // cur now points to the first child of the place  
    
    // temporary variables that contain data of the current place so this data can be returned
    // default values are assigned here
    ATerm Aname = ATparse("default_name");
    ATerm AinitialMarking = ATparse("0");
    ATerm Atype = ATparse("channel");
    
    // this loop goes through all the children of the <place>element
    // these children will be translated or ignored, this depends on the element name
    while (cur != NULL) {
      // current elements that are conceivable for translation are:
      // <name>  -  <initialMarking>  -  <type>
      // all other elements will be ignored in the translation
      
      if (!xmlStrcmp(cur->name, (const xmlChar *)"name")) {
	// the place contains a <name> element
	// a <name> element contains a childelement <text> which contains the name of the place
	// the name is retrieved below and assigned to Aname
	if (!(Aname=(ATerm)pn2gsRetrieveTextWithCheck(cur))) {
	  Aname = ATparse("default_name");
	}
	gsDebugMsg("    name: '%t'\n", Aname);
      } else if (!xmlStrcmp(cur->name, (const xmlChar *)"initialMarking")) {
	// the place contains an <initialMarking> element
	// this element contains a childelement <text> which contains the initial marking of the place
	// this marking is retrieved below and assigned to AinitialMarking
	
	if (!(AinitialMarking=pn2gsRetrieveText(cur))) {
	  AinitialMarking = ATparse("0");
	}
	if (atoi(ATwriteToString(AinitialMarking)) < 0) {
	  // if the initial marking is less than zero, it is resetted to zero
	  AinitialMarking = ATparse("0");
	  
	  gsWarningMsg("Place with id '%t' has initial marking is less than 0, resetting initial marking to 0! \n", Aid);
	}
	gsDebugMsg("    initialMarking: '%t'\n", AinitialMarking);
      } else if (!xmlStrcmp(cur->name, (const xmlChar *)"type")) {
	// the place contains an <type> element
	// this element contains a childelement <text> which contains the type of the place
	
	if (!(Atype=pn2gsRetrieveText(cur))) {
	  Atype = ATparse("channel");
	}
	if (!ATisEqual(Atype, ATparse("channel"))) {
	  // the type should either be omitted or have the value "channel"
	  // otherwise the place does not need to be translated!
	  gsWarningMsg("Place with id '%t' has type '%t' and will not be translated.\n", Aid, Atype);
	  return NULL;
	}
	gsDebugMsg("    type: '%t'\n", Atype);
      } else if (xmlNodeIsText(cur)) {
      } else {
	gsWarningMsg("Ignore an element named '%s'.\n", (const char *)cur->name);
      }
      cur = cur->next;
    }
    
    // argument order of returnvalue is id - name - initialMarking 
    return ATmakeAppl3(ATmakeAFun("place", 3, ATfalse), (ATerm)Aid, Aname, AinitialMarking);
  }
  
  //==================================================
  // pn2gsAterm_trans converts a pnml-transition to a usable ATerm
  //==================================================
  static ATermAppl pn2gsAterm_trans(xmlNodePtr cur) {
    // input: a pointer to the current transition
    // output: a usable translation of the transition,
    //         if the transition needs to be translated
    
    gsDebugMsg("> Start examining a transition...  \n");
    // first, we want to retrieve the id of the transition
    ATermAppl Aid;
    if (!xmlGetProp(cur, (const xmlChar *)"id")) {
      // the transition has NO id, so translation should be aborted!
      context.Abort = ATtrue;
      gsErrorMsg("A transition has no ID. \n");
      return NULL;
    } else {
      // the transition has an id, put it in Aid
      Aid = ATmakeAppl0(pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"id"))));
      
      if (strlen((const char *)xmlGetProp(cur, (const xmlChar *)"id")) > (global_MaxIDlength - global_TransChars)) {
	// check if the ID is not too long
	context.Abort = ATtrue;
	gsErrorMsg("The id: '%t' is too long! \n", Aid);
	return NULL;
      }
    }
    gsDebugMsg("    id: '%t'\n", Aid); 
    
    // second, we want to retrieve the necessary attributes of the transition
    cur = cur->xmlChildrenNode;
    // cur now points to the first child of the transition
    
    // temporary variables that contain data of the current transition so this data can be returned
    // default values are assigned here
    ATerm Aname = ATparse("default_name");
    ATerm Atype = ATparse("AND");
    
    // this loop goes through all the children of the <transition>element
    // these children will be translated or ignored, this depends on the element name
    while (cur != NULL) {
      // current elements that are conceivable for translation are:
      // <name>  -  <type>
      // all other elements will be ignored in the translation
      
      if (!xmlStrcmp(cur->name, (const xmlChar *)"name")) {
	// the transition contains a <name> element
	// a <name> element contains a childelement <text> which contains the name of the transition
	// the name is retrieved below and assigned to Aname
	if (!(Aname=(ATerm)pn2gsRetrieveTextWithCheck(cur))) {
	  Aname = ATparse("default_name");
	}
	gsDebugMsg("    name: '%t'\n", Aname);
      } else if (!xmlStrcmp(cur->name, (const xmlChar *)"type")) {
	// the transition contains an <type> element
	// this element contains a childelement <text> which contains the type of the transition
	if (!(Atype=pn2gsRetrieveText(cur))) {
	  Atype = ATparse("AND");
	}
	if (!ATisEqual(Atype, ATparse("AND"))) {
	  // the type should either be omitted or have the value "AND"
	  // otherwise the place does not need to be translated!
	  gsWarningMsg("Transition with id '%t' has type '%t' and will not be translated.\n", Aid, Atype);
	  return NULL;
	}
	gsDebugMsg("    type: '%t'\n", Atype);
      } else if (xmlNodeIsText(cur)) {
      } else {
	gsWarningMsg("Ignore an element named '%s'.\n", (const char *)cur->name);
      }
      cur = cur->next;
    }
    
    // argument order of returnvalue is id - name  
    return ATmakeAppl2(ATmakeAFun("transition", 2, ATfalse), (ATerm)Aid, Aname);
  }
  
  //==================================================
  // pn2gsAterm_arc converts a pnml-arc to a usable ATerm
  //==================================================
  static ATermAppl pn2gsAterm_arc(xmlNodePtr cur) {
    // input: a pointer to the current arc
    // output: a usable translation of the arc,
    //         if the arc needs to be translated
    
    gsDebugMsg("> Start examining an arc...  \n");
    // first, we want to retrieve the id of the arc
    ATermAppl Aid;
    if (!xmlGetProp(cur, (const xmlChar *)"id")) {
      // the arc has NO id, so translation should be aborted!
      context.Abort = ATtrue;
      gsErrorMsg("An arc has no ID. \n");
      return NULL;
    } else {
      // the arc has an id, put it in Aid
      Aid = ATmakeAppl0((pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"id")))));
      
      if (strlen((const char *)xmlGetProp(cur, (const xmlChar *)"id")) > (global_MaxIDlength - global_ArcChars)) {
	// check if the ID is not too long
	context.Abort = ATtrue;
	gsErrorMsg("The id: '%t' is too long! \n", Aid);
	return NULL;
      }
    }
    gsDebugMsg("    id: '%t'\n", Aid); 
    
    // second, we want to retrieve the source and the target of the arc
    ATermAppl Asource;
    if (!xmlGetProp(cur, (const xmlChar *)"source")) {
      // the arc has NO source, so the arc will not be translated!
      gsWarningMsg("Arc with id '%t' has no source and will not be translated.\n", Aid);
      return NULL;
    } else {
      Asource = ATmakeAppl0((pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"source")))));
    }
    gsDebugMsg("    source: '%t'\n", Asource);
    
    ATermAppl Atarget;
    if (!xmlGetProp(cur, (const xmlChar *)"target")) {
      // the arc has NO target, so the arc will not be translated!
      gsWarningMsg("Arc with id '%t' has no target and will not be translated.\n", Aid);
      return NULL;
    } else {
      Atarget = ATmakeAppl0(pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"target"))));
    }
    gsDebugMsg("    target: '%t'\n", Atarget);
    
    // third, we want to verify the arc needs translation (thus is of the correct type)
    cur = cur->xmlChildrenNode;
    // cur now points to the first child of the arc
    
    // temporary variables that contain data of the current arctype
    ATerm Atype = ATparse("some_strange`type=that n0b0dy u5e5...");
    
    // this loop goes through all the children of the <arc>element
    // these children will be examined or ignored, this depends on the element name
    while (cur != NULL) {
      // current elements that are conceivable for examination are:
      // <type>
      // all other elements will be ignored in the translation
      
      if (!xmlStrcmp(cur->name, (const xmlChar *)"type")) {
	// the arc contains a <type> element
	// this element contains a childelement <text> which contains the type of the transition
	
	if (!(Atype=pn2gsRetrieveText(cur))) {
	  Atype = ATparse("some_strange`type=that n0b0dy u5e5...");
	}
	if (!ATisEqual(Atype, ATparse("some_strange`type=that n0b0dy u5e5..."))) {
	  // the type should be omitted
	  // otherwise the arc does not need to be translated!
	  gsWarningMsg("Arc with id '%t' has type '%t' and will not be translated.\n", Aid, Atype);
	  return NULL;
	}
	gsDebugMsg("    type: '%t'\n", Atype);
      } else if (xmlNodeIsText(cur)) {
      } else {
	gsWarningMsg("Ignore an element named '%s'.\n", (const char *)cur->name);
      }
      cur = cur->next;
    }
    
    // argument order of returnvalue is id - source - target
    return ATmakeAppl3(ATmakeAFun("arc", 3, ATfalse), (ATerm)Aid, (ATerm)Asource, (ATerm)Atarget);
  }
  
  /*                        */
  /*                        */
  /*                        */
  /* Read-in phase function */
  /*                        */
  /*                        */
  /*                        */

  //==================================================
  // pn2gsAterm converts the pnml-input to a usable ATerm
  //==================================================
  static ATermAppl pn2gsAterm(xmlDocPtr doc) {
    // input: a pointer of the type xmlDocPtr which points to the parsed XML-file
    // output: an ATermAppl, translated from the XML-file,
    //         in which only relevant elements/attributes are concluded
    
    //==================================================
    // initializations and initial checks.
    //==================================================
    xmlNodePtr cur = xmlDocGetRootElement(doc);
    if (cur == NULL) {
      gsErrorMsg("File is empty. \n");
      return NULL;
    }
    if (xmlStrcmp(cur->name, (const xmlChar *)"pnml")) {
      gsErrorMsg("File is not a PNML file!  \n");
      return NULL;
    }
    // cur now points to the <pnml>element
    
    cur = cur->xmlChildrenNode;
    // the first <net>element, if any present, is selected by cur
    while (cur != NULL && xmlStrcmp(cur->name, (const xmlChar *)"net")) {
      if (!xmlNodeIsText(cur)) {
	gsWarningMsg("Element '%s' is not a Petri net and will be ignored (including it's sub-elements).\n",(const char *)cur->name);
      }
      cur = cur->next;
    }
    if (cur == NULL) {
      gsErrorMsg("File does not contain a Petri net. \n");
      return NULL;
    }   
    
    // cur now points to the first <net>element
    gsDebugMsg("\nStart converting the Petri net to an ATerm...  \n \n");
    
    //==================================================
    // actual translation starts here
    //==================================================
    // retrieve the ID of the Petri net
    ATermAppl ANetID;
    if (!xmlGetProp(cur, (const xmlChar *)"id")) {
      ANetID = ATmakeAppl0(ATmakeAFun("Net_Petri_net", 0, ATtrue));
      gsWarningMsg("NO NET-ID FOUND!\n");
    } else {
      // the net has an id, put it in ANetID
      AFun NetID = pn2gsCheckAFun(ATmakeAFunId((char *)xmlGetProp(cur, (const xmlChar *)"id")));
      NetID = ATprependAFun("Net_",NetID);
      ANetID = ATmakeAppl0(NetID);
    }
    gsDebugMsg("NetID = '%t'\n",ANetID);
    
    cur = cur->xmlChildrenNode;
    // cur now points to the first element in the Petri net
    
    // lists of the places, transitions and arcs that will be translated
    ATermList APlaces=ATmakeList0();
    ATermList ATransitions=ATmakeList0();
    ATermList AArcs=ATmakeList0();
    
    // temporary variables that contain data of the current place
    // so this data can be inserted in APlaces
    ATermAppl ACurrentPlace;
    // temporary variables that contain data of the current transition
    // so this data can be inserted in ATransitions
    ATermAppl ACurrentTransition;
    // temporary variables that contain data of the current arc
    // so this data can be inserted in AArcs
    ATermAppl ACurrentArc;
    
    // if an element has no ID, Abort is set to ATtrue
    // if Abort == ATtrue, the translation is aborted!
    context.Abort = ATfalse;
    
    // this loop goes through all the children of the <net>element
    // these children will be translated or ignored, this depends on the element name
    while (cur != NULL) {
      // current elements that are conceivable for translation are:
      // <place>  -  <transition>  -  <arc>
      // all other elements will be ignored in the translation
      
      if (!xmlStrcmp(cur->name, (const xmlChar *)"place")) {
	if (!(ACurrentPlace=pn2gsAterm_place(cur))) {
	  // pn2gsAterm_place returns NULL, so the place will not be translated.
	  if (context.Abort == ATtrue) {
	    // pn2gsAterm_place has set context.Abort to ATtrue
	    // this means the place had no ID or the ID is too long (> global_MaxIDlength - 6)
	    // therefor the translation will be aborted!
	    return NULL;
	  }
	} else {
	  APlaces = ATinsert(APlaces, (ATerm)ACurrentPlace);
	  gsDebugMsg("  Translate this place: %t\n", (ATerm)ACurrentPlace);
	}
      } else if (!xmlStrcmp(cur->name, (const xmlChar *)"transition")) {
	if(!(ACurrentTransition=pn2gsAterm_trans(cur))) {
	  // pn2gsAterm_trans returns NULL, so the transition will not be translated.
	  if (context.Abort == ATtrue) {
	    // pn2gsAterm_trans has set context.Abort to ATtrue
	    // this means the transition had no ID or the ID is too long (> global_MaxIDlength - 6)
	    // therefor the translation will be aborted!
	    return NULL;
	  }
	} else {
	  ATransitions = ATinsert(ATransitions, (ATerm)ACurrentTransition);
	  gsDebugMsg("  Translate this transition: %t\n", (ATerm)ACurrentTransition);
	}
      } else if (!xmlStrcmp(cur->name, (const xmlChar *)"arc")) {
	if(!(ACurrentArc=pn2gsAterm_arc(cur))) {
	  // pn2gsAterm_arc returns NULL, so the arc will not be translated.
	  if (context.Abort == ATtrue) {
	    // pn2gsAterm_arc has set context.Abort to ATtrue
	    // this means the arc had no ID or the ID is too long (> global_MaxIDlength - 6)
	    // therefor the translation will be aborted!
	    return NULL;
	  }
	} else {
	  AArcs = ATinsert(AArcs, (ATerm)ACurrentArc);
	  gsDebugMsg("  Translate this arc: %t\n", (ATerm)ACurrentArc);
	}
      } else if (xmlNodeIsText(cur)) {
      } else {
	gsWarningMsg("An element named '%s' will be ignored in the translation (including it's sub-elements).\n",(const char *)cur->name);
      };
      cur = cur->next;
    };
    
    gsDebugMsg("\nConversion of PNML to ATerm succesfully completed. \n");
    
    // argument order of returnvalue is places - transitions - arcs - NetID
    return ATmakeAppl4(ATmakeAFun("PetriNet", 4, ATtrue), (ATerm)ATreverse(APlaces), (ATerm)ATreverse(ATransitions), (ATerm)ATreverse(AArcs), (ATerm)ANetID);
  }

  /*                                    */
  /*                                    */
  /*                                    */
  /* Functions for the generation phase */
  /*                                    */
  /*                                    */
  /*                                    */

  //==================================================
  // pn2gsGenerateAction_trans generates the action t_..._mon for a given transition
  //==================================================
  static ATermAppl pn2gsGenerateAction_trans(char * CurrentId, char * CurrentName){
    // input: access to the context + id and name of the transition
    // output: the action t_transid_mon or t_transid_transname_mon

      char Trans[global_MaxIDlength] = "t_";
      if (strcmp(CurrentName, "default_name")) {
	// name of the transition is not "default_name"
	// name of the transition may be used
	if ((strlen(CurrentName) + strlen(CurrentId)) > (global_MaxIDlength - (global_TransChars + 1))) {
	  // name + id are too long; only use the id
	  // make the action: t_transid_mon
	  CurrentId = strcat(strcat(Trans, CurrentId), "_mon");
	} else {
	  // name + id are of good length; use both
	  // make the action: t_transid_transname_mon
	  CurrentId = strcat(strcat(strcat(strcat(Trans, CurrentId), "_"), CurrentName), "_mon");
	}
      } else {
	// name of the transition is "default_name"
	// name of the transition will not be used
	
	// make the action: t_transid_mon
	CurrentId = strcat(strcat(Trans, CurrentId), "_mon");
      }
      return ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
  }
  
  //==================================================
  // pn2gsGenerateActions generates all the GenSpect actions
  //==================================================
  static ATermList pn2gsGenerateActions(void){
    // input: access to the context
    // output: an ATermList of <ActID>'s
    
    // #actions = 3x #arcs + 1x #transitions
    // the possible actions will be stored in ActionList
    ATermList ActionsList = ATmakeList0();
    
    // the possible actions are
    // for each arc:                                  arcid     >-<     _arcid     >-<     __arcid
    // for each transition (if a name is present):    t_transid_transname_mon
    // for each transition (if no name is present):   t_transid_mon
    
    // variables to store the Current Action to be inserted into ActionsList
    ATermAppl CurrentAction; 
    char * CurrentId;
    
    // variable to go through all the arc_in-ids, the arc_out-ids and the transition-ids
    ATermList Ids;
    
    //==================================================
    // create actions from context.arc_in
    //==================================================
    Ids = ATtableKeys(context.arc_in);
    while (ATisEmpty(Ids) == ATfalse) {
      // make the action: arcID
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(Ids)));
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      // make the action: _arcID
      char Underscore0[global_MaxIDlength] = "_";     
      CurrentId = strcat(Underscore0, CurrentId);
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      // make the action: __arcID
      char Underscore1[global_MaxIDlength] = "_";
      CurrentId = strcat(Underscore1, CurrentId);
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      
      Ids = ATgetNext(Ids);
    }
    
    //==================================================
    // create actions from context.arc_out
    //==================================================
    Ids = ATtableKeys(context.arc_out);
    while (ATisEmpty(Ids) == ATfalse) {
      // make the action: arcID
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(Ids)));
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      // make the action: _arcID
      char Underscore0[global_MaxIDlength] = "_";     
      CurrentId = strcat(Underscore0, CurrentId);
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      // make the action: __arcID
      char Underscore1[global_MaxIDlength] = "_";
      CurrentId = strcat(Underscore1, CurrentId);
      CurrentAction = ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      
      Ids = ATgetNext(Ids);
    }
    
    //==================================================
    // create actions from the transitions
    //==================================================
    // All transitions have a name. If no name is defined in PNML, it is "default_name"
    Ids = ATtableKeys(context.trans_name);
    while (ATisEmpty(Ids) == ATfalse) {
      
      CurrentAction = pn2gsGenerateAction_trans(ATgetName(ATgetAFun(ATgetFirst(Ids))), ATgetName(ATgetAFun(ATtableGet(context.trans_name, ATgetFirst(Ids)))));
      ActionsList = ATinsert(ActionsList, (ATerm)gsMakeActId(CurrentAction, ATmakeList0()));
      gsDebugMsg("Action: %t created.\n", CurrentAction);
      Ids = ATgetNext(Ids);
    }

    //now reverse the whole thing
    return ATreverse(ActionsList);
  }

  //==================================================
  // pn2gsSyncIn creates the GenSpect syncronistation of all elements of the list
  //==================================================
  static ATermAppl pn2gsSyncIn(ATermList Ids){
    // input: a not-empty list with the ID's
    // output: an AtermAppl that is the SYNC of all the ID's in the input-list

    ATermAppl sync;
    char * CurrentElement = ATgetName(ATgetAFun(ATgetFirst(Ids)));
    if (ATgetLength(Ids) > 1) {
      sync = gsMakeSync(pn2gsSyncIn(ATgetNext(Ids)), gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0()));
    } else {
      // ATgetLength(Ids) == 1, since the input-list is not-empty!
      sync = gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0());
    }
    return sync;
  }

  //==================================================
  // pn2gsSyncOut creates the GenSpect syncronistation of all elements of the list, preceded by an underscore
  //==================================================
  static ATermAppl pn2gsSyncOut(ATermList Ids){
    // input: a not-empty list with the ID's
    // output: an AtermAppl that is the SYNC of all the ID's in the input-list, each ID preceded by an underscore

    ATermAppl sync;
    char Underscore[global_MaxIDlength] = "_";     
    char * CurrentElement = ATgetName(ATgetAFun(ATgetFirst(Ids)));
    CurrentElement = strcat(Underscore, CurrentElement);
    if (ATgetLength(Ids) > 1) {
      sync = gsMakeSync(pn2gsSyncOut(ATgetNext(Ids)), gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0()));
    } else {
      // ATgetLength(Ids) == 1, since the input-list is not-empty!
      sync = gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0());
    }
    return sync;
  }

  //==================================================
  // pn2gsGenerateTransition generates the GenSpect Process Equations belonging to one transition
  //==================================================
  static ATermList pn2gsGenerateTransition(ATerm TransID){
    // input: access to the context; ID of the transtion
    // output: an updated list of the processes: the processes belonging to the transition added to the list

    ATermList EquationList=ATmakeList0();

    // variables to store the name and id of the transition
    char * CurrentTransId = ATgetName(ATgetAFun(TransID)) ;
    char * CurrentTransName = ATgetName(ATgetAFun(ATtableGet(context.trans_name, TransID)));

    // variables to store the process names
    ATermAppl CurrentTrans;
    ATermAppl CurrentTransIn;
    ATermAppl CurrentTransOut;
    char Trans0[global_MaxIDlength] = "T_";
    char Trans1[global_MaxIDlength] = "T_";
    char Trans2[global_MaxIDlength] = "T_";
    
    //==================================================
    // retrieve the process names
    //==================================================
    if (strcmp(CurrentTransName, "default_name")) {
      // name of the transition is not "default_name"
      // name of the transition may be used
      if ((strlen(CurrentTransName) + strlen(CurrentTransId)) > (global_MaxIDlength - (global_TransChars + 1))) {
	// name + id are too long; only use the id
	gsWarningMsg("The combination of ID: '%s' and name: '%s' is too long, so the name is left out.\n", CurrentTransId, CurrentTransName);
	// make the processes: T_transid     >-<     T_transid_in     >-<     T_transid_out
	CurrentTrans = ATmakeAppl0(ATmakeAFun(strcat(Trans0, CurrentTransId), 0, ATtrue));
	CurrentTransIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(Trans1, CurrentTransId), "_in"), 0, ATtrue));
	CurrentTransOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(Trans2, CurrentTransId), "_out"), 0, ATtrue));
      } else {
	// name + id are of good length; use both
	// make the processes: T_transid_transname     >-<     T_transid_transname_in     >-<     T_transid_transname_out
	CurrentTrans = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(Trans0, CurrentTransId), "_"), CurrentTransName), 0, ATtrue));
	CurrentTransIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Trans1, CurrentTransId), "_"), CurrentTransName), "_in"), 0, ATtrue));
	CurrentTransOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Trans2, CurrentTransId), "_"), CurrentTransName), "_out"), 0, ATtrue));
      }
    } else {
      // name of the transition is "default_name"
      // name of the transition will not be used
      // make the processes: T_transid     >-<     T_transid_in     >-<     T_transid_out
      CurrentTrans = ATmakeAppl0(ATmakeAFun(strcat(Trans0, CurrentTransId), 0, ATtrue));
      CurrentTransIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(Trans1, CurrentTransId), "_in"), 0, ATtrue));
      CurrentTransOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(Trans2, CurrentTransId), "_out"), 0, ATtrue));
    }

    // insert the name of the process T_ti into context.transitions
    // this is needed for the generation of the general process Trans
    context.transitions = ATinsert(context.transitions, (ATerm)CurrentTrans);
    gsDebugMsg("context.transitions now contains the following transitions: %t\n", context.transitions);

    //==================================================
    // generate the processes
    //==================================================
    ATermAppl Process;
    ATermAppl MonitorAction = pn2gsGenerateAction_trans(CurrentTransId, CurrentTransName);
    if (!(ATtableGet(context.trans_in, TransID)) && !(ATtableGet(context.trans_out, TransID))) {
      // Transition has no incoming arcs and no outgoing arcs
      gsDebugMsg("No in and no out for trans: %t\n", (ATerm)CurrentTrans);
      // create T_ti
      Process = gsMakeActionProcess(MonitorAction, ATmakeList0());
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTrans, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTrans);
      
    } else if (ATtableGet(context.trans_in, TransID) && !(ATtableGet(context.trans_out, TransID))) {
      // Transition has incoming arcs but no outgoing arcs
      gsDebugMsg("In and no out for trans: %t\n", (ATerm)CurrentTrans);
      // create T_ti
      Process = gsMakeSync(gsMakeActionProcess(CurrentTransIn, ATmakeList0()), gsMakeActionProcess(MonitorAction, ATmakeList0()));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTrans, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTrans);
      // create T_ti_in
      Process = pn2gsSyncIn((ATermList)ATtableGet(context.trans_in, TransID));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTransIn, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTransIn);
      
    } else if (!(ATtableGet(context.trans_in, TransID)) && ATtableGet(context.trans_out, TransID)) {
      // Transition has outgoing arcs but no incoming arcs
      gsDebugMsg("Out and no in for trans: %t\n", (ATerm)CurrentTrans);
      // create T_ti
      Process = gsMakeSync(gsMakeActionProcess(CurrentTransOut, ATmakeList0()), gsMakeActionProcess(MonitorAction, ATmakeList0()));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTrans, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTrans);
      // create T_ti_out
      Process = pn2gsSyncOut((ATermList)ATtableGet(context.trans_out, TransID));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTransOut, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTransOut);
      
    } else if (ATtableGet(context.trans_in, TransID) && ATtableGet(context.trans_out, TransID)) {
      // Transition has incoming arcs and outgoing arcs
      gsDebugMsg("In and out for trans: %t\n", (ATerm)CurrentTrans);
      // create T_ti
      Process = gsMakeSync(gsMakeSync(gsMakeActionProcess(CurrentTransIn, ATmakeList0()), gsMakeActionProcess(CurrentTransOut, ATmakeList0())), gsMakeActionProcess(MonitorAction, ATmakeList0()));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTrans, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTrans);
      // create T_ti_in
      Process = pn2gsSyncIn((ATermList)ATtableGet(context.trans_in, TransID));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTransIn, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTransIn);
      // create T_ti_out
      Process = pn2gsSyncOut((ATermList)ATtableGet(context.trans_out, TransID));
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentTransOut, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentTransOut);
    }
    return EquationList;
  }

  //==================================================
  // pn2gsChoiceIn creates the GenSpect choice of all elements of the list
  //==================================================
  static ATermAppl pn2gsChoiceIn(ATermList Ids){
    // input: a not-empty list with the ID's
    // output: an AtermAppl that is the CHOICE of all the ID's in the input-list

    ATermAppl choice;
    char * CurrentElement = ATgetName(ATgetAFun(ATgetFirst(Ids)));
    if (ATgetLength(Ids) > 1) {
      choice = gsMakeChoice(pn2gsChoiceIn(ATgetNext(Ids)), gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0()));
    } else {
      // ATgetLength(Ids) == 1, since the input-list is not-empty!
      choice = gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0());
    }
    return choice;
  }

  //==================================================
  // pn2gsChoiceOut creates the GenSpect choice of all elements of the list, preceded by an underscore
  //==================================================
  static ATermAppl pn2gsChoiceOut(ATermList Ids){
    // input: a not-empty list with the ID's
    // output: an AtermAppl that is the CHOICE of all the ID's in the input-list, each ID preceded by an underscore

    ATermAppl choice;
    char Underscore[global_MaxIDlength] = "_";
    char * CurrentElement = ATgetName(ATgetAFun(ATgetFirst(Ids)));
    CurrentElement = strcat(Underscore, CurrentElement);
    if (ATgetLength(Ids) > 1) {
      choice = gsMakeChoice(pn2gsChoiceOut(ATgetNext(Ids)), gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0()));
    } else {
      // ATgetLength(Ids) == 1, since the input-list is not-empty!
      choice = gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList0());
    }
    return choice;
  }

  //==================================================
  // pn2gsPlaceParameterNat generates the parameter for a given place, of type Nat
  //==================================================
  static ATermAppl pn2gsPlaceParameterNat(char * PlaceName) {
    // input: the name of a place
    // output: the parameter of that place, or type Nat
    //         named: pm_PlaceName

    ATermAppl CurrentParameter;
    char Param[global_MaxIDlength] = "pm_";
    CurrentParameter = gsMakeDataVarId(ATmakeAppl0(ATmakeAFun(strcat(Param, PlaceName), 0, ATtrue)), gsMakeSortIdNat());
    return CurrentParameter;
  }

  //==================================================
  // pn2gsPlaceParameterPos generates the parameter for a given place, of type Pos
  //==================================================
  static ATermAppl pn2gsPlaceParameterPos(char * PlaceName) {
    // input: the name of a place
    // output: the parameter of that place, or type Pos
    //         named: pm_PlaceName

    ATermAppl CurrentParameter;
    char Param[global_MaxIDlength] = "pm_";
    CurrentParameter = gsMakeDataVarId(ATmakeAppl0(ATmakeAFun(strcat(Param, PlaceName), 0, ATtrue)), gsMakeSortIdPos());
    return CurrentParameter;
  }

  //==================================================
  // pn2gsGeneratePlace generates the GenSpect Process Equations belonging to one place
  //==================================================
  static ATermList pn2gsGeneratePlace(ATerm PlaceID){
    // input: access to the context; ID of the place
    // output: an updated list of the processes: the processes belonging to the place added to the list

    ATermList EquationList=ATmakeList0();

    // variables to store the name and id of the place
    char * CurrentPlaceId = ATgetName(ATgetAFun(PlaceID));
    char * CurrentPlaceName = ATgetName(ATgetAFun(ATtableGet(context.place_name, PlaceID)));

    // variables to store the process names
    ATermAppl CurrentPlace;
    ATermAppl CurrentPlaceAdd;
    ATermAppl CurrentPlaceIn;
    ATermAppl CurrentPlaceRem;
    ATermAppl CurrentPlaceOut;
    char Place0[global_MaxIDlength] = "P_";
    char Place1[global_MaxIDlength] = "P_";
    char Place2[global_MaxIDlength] = "P_";
    char Place3[global_MaxIDlength] = "P_";
    char Place4[global_MaxIDlength] = "P_";
    
    //==================================================
    // retrieve the process names
    //==================================================
    if (strcmp(CurrentPlaceName, "default_name")) {
      // name of the place is not "default_name"
      // name of the place may be used
      if ((strlen(CurrentPlaceName) + strlen(CurrentPlaceId)) > (global_MaxIDlength - (global_PlaceChars + 1))) {
	// name + id are too long; only use the id
	gsWarningMsg("The combination of ID: '%s' and name: '%s' is too long, so the name is left out.\n", CurrentPlaceId, CurrentPlaceName);
	// make the processes: P_placeid     >-<     P_placeid_add     >-<     P_placeid_in     >-<     P_placeid_rem     >-<     P_placeid_out
	CurrentPlace = ATmakeAppl0(ATmakeAFun(strcat(Place0, CurrentPlaceId), 0, ATtrue));
	CurrentPlaceAdd = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place1, CurrentPlaceId), "_add"), 0, ATtrue));
	CurrentPlaceIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place2, CurrentPlaceId), "_in"), 0, ATtrue));
	CurrentPlaceRem = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place3, CurrentPlaceId), "_rem"), 0, ATtrue));
	CurrentPlaceOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place4, CurrentPlaceId), "_out"), 0, ATtrue));
      } else {
	// name + id are of good length; use both
	// make the processes: P_placeid_placename     >-<     P_placeid_placename_add     >-<     P_placeid_placename_in     >-<
	//                     P_placeid_placename_rem     >-<     P_placeid_placename_out
	CurrentPlace = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(Place0, CurrentPlaceId), "_"), CurrentPlaceName), 0, ATtrue));
	CurrentPlaceAdd = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Place1, CurrentPlaceId), "_"), CurrentPlaceName), "_add"), 0, ATtrue));
	CurrentPlaceIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Place2, CurrentPlaceId), "_"), CurrentPlaceName), "_in"), 0, ATtrue));
	CurrentPlaceRem = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Place3, CurrentPlaceId), "_"), CurrentPlaceName), "_rem"), 0, ATtrue));
	CurrentPlaceOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(strcat(strcat(Place4, CurrentPlaceId), "_"), CurrentPlaceName), "_out"), 0, ATtrue));
      }
    } else {
      // name of the transition is "default_name"
      // name of the transition will not be used
      // make the processes: P_placeid     >-<     P_placeid_add     >-<     P_placeid_in     >-<     P_placeid_rem     >-<     P_placeid_out
      CurrentPlace = ATmakeAppl0(ATmakeAFun(strcat(Place0, CurrentPlaceId), 0, ATtrue));
      CurrentPlaceAdd = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place1, CurrentPlaceId), "_add"), 0, ATtrue));
      CurrentPlaceIn = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place2, CurrentPlaceId), "_in"), 0, ATtrue));
      CurrentPlaceRem = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place3, CurrentPlaceId), "_rem"), 0, ATtrue));
      CurrentPlaceOut = ATmakeAppl0(ATmakeAFun(strcat(strcat(Place4, CurrentPlaceId), "_out"), 0, ATtrue));
    }

    // insert the name of the process P_pi into context.places
    // this is needed for the generation of the general process PetriNet
    context.places = ATinsert(context.places, (ATerm)CurrentPlace);
    gsDebugMsg("context.places now contains the following places: %t\n", context.places);

    //==================================================
    // retrieve the maximum concurrency
    //==================================================
    int MaxConcIn_int;
    if (!(ATtableGet(context.place_in, PlaceID))) {
      MaxConcIn_int = 0;
    } else {
      MaxConcIn_int = ATgetLength((ATermList)ATtableGet(context.place_in, PlaceID));
    }
    ATermAppl MaxConcIn;
    MaxConcIn = gsMakeNumber(ATmakeAppl0(ATmakeAFunInt0(MaxConcIn_int)), gsMakeSortIdNat());
    if (gsIsNumber(MaxConcIn)) {
     gsDebugMsg("Parameter %t is a Number\n", MaxConcIn);
    } else {
      gsDebugMsg("Parameter %t is not a Number\n", MaxConcIn);
    }

    int MaxConcOut_int;
    if (!(ATtableGet(context.place_out, PlaceID))) {
      MaxConcOut_int = 0;
    } else {
      MaxConcOut_int = ATgetLength((ATermList)ATtableGet(context.place_out, PlaceID));
    }
    ATermAppl MaxConcOut;
    MaxConcOut = gsMakeNumber(ATmakeAppl0(ATmakeAFunInt0(MaxConcOut_int)), gsMakeSortIdNat());
    if (gsIsNumber(MaxConcIn)) {
      gsDebugMsg("Parameter %t is a Number\n", MaxConcOut);
    } else {
      gsDebugMsg("Parameter %t is not a Number\n", MaxConcOut);
    }
    gsDebugMsg("Place %t has maximum concurrency in: '%d' and out: '%d'\n", PlaceID, MaxConcIn_int, MaxConcOut_int);
    gsDebugMsg("Place %t has maximum concurrency in: '%t' and out: '%t'\n", PlaceID, MaxConcIn, MaxConcOut);

    //==================================================
    // generate the processes
    //==================================================
    {
      /* Creation of P_pi */
      ATermAppl ProcVar;
      ATermAppl Process;
      ATermAppl SubProcess0;
      ATermAppl CondIf0;
      ATermAppl CondThan0;
      ATermAppl SubProcess1;
      ATermAppl CondIf1;
      ATermAppl CondThan1;
      ATermAppl SubProcess2;
      ATermAppl CondIf2;
      ATermAppl CondThan2;
      ATermAppl SumVar0 = gsMakeDataVarId(ATmakeAppl0(ATmakeAFun("y", 0, ATtrue)), gsMakeSortIdPos());
      ATermAppl SumVar1 = gsMakeDataVarId(ATmakeAppl0(ATmakeAFun("z", 0, ATtrue)), gsMakeSortIdPos());
      ATermList SumVars;
      gsDebugMsg("Parameter %t is %d a DataVarId\n", SumVar0, gsIsDataVarId(SumVar0));
      gsDebugMsg("Parameter %t is %d a DataVarId\n", SumVar1, gsIsDataVarId(SumVar1));

      // create P_pi parameter
      ProcVar = pn2gsPlaceParameterNat(gsATermAppl2String(CurrentPlace));
      gsDebugMsg("Parameter %t is %d a DataVarId\n", ProcVar, gsIsDataVarId(ProcVar));

      // create first sum-sub-process
      SumVars = ATmakeList1((ATerm)SumVar0);
      CondIf0 = gsMakeDataExprLTE(gsMakeDataExprPos2Nat(SumVar0), MaxConcIn);
      CondThan0 = gsMakeSeq(gsMakeActionProcess(CurrentPlaceAdd, ATmakeList1((ATerm)SumVar0)), gsMakeActionProcess(CurrentPlace, ATmakeList1((ATerm)gsMakeDataExprAdd(gsMakeDataExprPos2Nat(SumVar0), ProcVar))));
      SubProcess0 = gsMakeSum(SumVars, gsMakeCond(CondIf0, CondThan0, gsMakeDelta()));
      gsDebugMsg("Parameter %t is %d a Sum\n", SubProcess0, gsIsSum(SubProcess0));

      // create second sum-sub-process
      SumVars = ATmakeList1((ATerm)SumVar1);
      CondIf1 = gsMakeDataExprLTE(gsMakeDataExprPos2Nat(SumVar1), gsMakeDataExprMin(MaxConcOut, ProcVar));
      CondThan1 = gsMakeSeq(gsMakeActionProcess(CurrentPlaceRem, ATmakeList1((ATerm)SumVar1)), gsMakeActionProcess(CurrentPlace, ATmakeList1((ATerm)gsMakeDataExprMax(gsMakeNumber(ATmakeAppl0(ATmakeAFun("0", 0, ATtrue)), gsMakeSortIdInt()), gsMakeDataExprSubt(ProcVar, gsMakeDataExprPos2Nat(SumVar1))))));
      SubProcess1 = gsMakeSum(SumVars, gsMakeCond(CondIf1, CondThan1, gsMakeDelta()));
      gsDebugMsg("Parameter %t is %d a Sum\n", SubProcess1, gsIsSum(SubProcess1));

      // create third sum-sub-process
      SumVars = ATmakeList2((ATerm)SumVar0, (ATerm)SumVar1);
      CondIf2 = gsMakeDataExprAnd(CondIf0, CondIf1);
      CondThan2 = gsMakeSeq(gsMakeSync(gsMakeActionProcess(CurrentPlaceAdd, ATmakeList1((ATerm)SumVar0)), gsMakeActionProcess(CurrentPlaceRem, ATmakeList1((ATerm)SumVar1))), gsMakeActionProcess(CurrentPlace, ATmakeList1((ATerm)gsMakeDataExprMax(gsMakeNumber(ATmakeAppl0(ATmakeAFun("0", 0, ATtrue)), gsMakeSortIdInt()), gsMakeDataExprSubt(gsMakeDataExprAdd(gsMakeDataExprPos2Nat(SumVar0),ProcVar), gsMakeDataExprPos2Nat(SumVar1))))));
      SubProcess2 = gsMakeSum(SumVars, gsMakeCond(CondIf2, CondThan2, gsMakeDelta()));
      gsDebugMsg("Parameter %t is %d a Sum\n", SubProcess2, gsIsSum(SubProcess2));

      // create P_pi
      Process = gsMakeChoice(gsMakeChoice(SubProcess0, SubProcess1), SubProcess2);
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentPlace, ATmakeList0()), ATmakeList1((ATerm)ProcVar), Process));
      gsDebugMsg("Process: %t created.\n", CurrentPlace);
    }

    {
      /* Creation of P_pi_add */
      ATermAppl ProcVar = pn2gsPlaceParameterPos(gsATermAppl2String(CurrentPlaceAdd));
      ATermAppl CondIf = gsMakeDataExprGT(ProcVar, gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdPos()));
      ATermAppl CondThan = gsMakeSync(gsMakeActionProcess(CurrentPlaceIn, ATmakeList0()), gsMakeActionProcess(CurrentPlaceAdd, ATmakeList1((ATerm)gsMakeDataExprMax(gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdInt()), gsMakeDataExprSubt(ProcVar, gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdPos()))))));
      ATermAppl CondElse = gsMakeActionProcess(CurrentPlaceIn, ATmakeList0());
      ATermAppl Process = gsMakeCond(CondIf, CondThan, CondElse);
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentPlaceAdd, ATmakeList0()), ATmakeList1((ATerm)ProcVar), Process));
      gsDebugMsg("Process: %t created.\n", CurrentPlaceAdd);
    }

    {
      /* Creation of P_pi_in */
      ATermAppl Process;
      if (MaxConcIn_int == 0) {
	Process = gsMakeTau();
      } else {
	Process = pn2gsChoiceIn((ATermList)ATtableGet(context.place_in, PlaceID));
      }
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentPlaceIn, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentPlaceIn);
    }

    {
      /* Creation of P_pi_rem */
      ATermAppl ProcVar = pn2gsPlaceParameterPos(gsATermAppl2String(CurrentPlaceRem));
      ATermAppl CondIf = gsMakeDataExprGT(ProcVar, gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdPos()));
      ATermAppl CondThan = gsMakeSync(gsMakeActionProcess(CurrentPlaceOut, ATmakeList0()), gsMakeActionProcess(CurrentPlaceRem, ATmakeList1((ATerm)gsMakeDataExprMax(gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdInt()), gsMakeDataExprSubt(ProcVar, gsMakeNumber(ATmakeAppl0(ATmakeAFun("1", 0, ATtrue)), gsMakeSortIdPos()))))));
      ATermAppl CondElse = gsMakeActionProcess(CurrentPlaceOut, ATmakeList0());
      ATermAppl Process = gsMakeCond(CondIf, CondThan, CondElse);
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentPlaceRem, ATmakeList0()), ATmakeList1((ATerm)ProcVar), Process));
      gsDebugMsg("Process: %t created.\n", CurrentPlaceRem);
    }

    {
      /* Creation of P_pi_out */
      ATermAppl Process;
      if (MaxConcOut_int == 0) {
	Process = gsMakeTau();
      } else {
	Process = pn2gsChoiceOut((ATermList)ATtableGet(context.place_out, PlaceID));
      }
      EquationList = ATinsert(EquationList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(CurrentPlaceOut, ATmakeList0()), ATmakeList0(), Process));
      gsDebugMsg("Process: %t created.\n", CurrentPlaceOut);
    }

    // return the updated list of the processes: the processes belonging to the place added to the input-list
    return EquationList;
  }

  //==================================================
  // pn2gsHideList generates a list with all the actions that should be hidden
  //==================================================
  static ATermList pn2gsHideList(ATermList List0, ATermList List1) {
    // input: two lists, each containing different elements
    // output: a list in which all the elements of both inputed lists are present, preceded by two underscores.

    ATermList ReturnList = ATmakeList0();
    char * CurrentId;
    while (ATisEmpty(List0) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List0)));
      char Underscores[global_MaxIDlength] = "__";
      CurrentId = strcat(Underscores, CurrentId);
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      List0 = ATgetNext(List0);
    }
    while (ATisEmpty(List1) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List1)));
      char Underscores[global_MaxIDlength] = "__";
      CurrentId = strcat(Underscores, CurrentId);
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      List1 = ATgetNext(List1);
    }
    return ReturnList;
  }

  //==================================================
  // pn2gsRestrictList generates a list with all the actions that should be restricted
  //==================================================
  static ATermList pn2gsRestrictList(ATermList List0, ATermList List1) {
    // input: two lists, each containing different elements
    // output: a list in which all the elements of both inputed lists are present twice, once preceded by an underscore.

    ATermList ReturnList = ATmakeList0();
    char * CurrentId;
    while (ATisEmpty(List0) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List0)));
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      char Underscores[global_MaxIDlength] = "_";
      CurrentId = strcat(Underscores, CurrentId);
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      List0 = ATgetNext(List0);
    }
    while (ATisEmpty(List1) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List1)));
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      char Underscores[global_MaxIDlength] = "_";
      CurrentId = strcat(Underscores, CurrentId);
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeActId(ATmakeAppl0(ATmakeAFun(CurrentId, 0, ATtrue)), ATmakeList0()));
      List1 = ATgetNext(List1);
    }
    return ReturnList;
  }

  //==================================================
  // pn2gsCommList generates a list with all the actions that communicate
  //==================================================
  static ATermList pn2gsCommList(ATermList List0, ATermList List1) {
    // input: two lists, each containing different elements
    // output: a list in which all the elements of both inputed lists 
    //         communicate with itself, once preceded by an underscore
    //         to itself, preceded by two underscores.

    ATermList ReturnList = ATmakeList0();
    char * CurrentId;
    while (ATisEmpty(List0) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List0)));
      char Underscore[global_MaxIDlength] = "_";
      char Underscores[global_MaxIDlength] = "__";
      ATermAppl CommExpr = gsMakeCommExpr(gsMakeMultActName(ATmakeList2((ATerm)gsString2ATermAppl(CurrentId), (ATerm)gsString2ATermAppl(strcat(Underscore, CurrentId)))), gsString2ATermAppl(strcat(Underscores, CurrentId)));
      ReturnList = ATinsert(ReturnList, (ATerm)CommExpr);
      List0 = ATgetNext(List0);
    }
    while (ATisEmpty(List1) == ATfalse) {
      CurrentId = ATgetName(ATgetAFun(ATgetFirst(List1)));
      char Underscore[global_MaxIDlength] = "_";
      char Underscores[global_MaxIDlength] = "__";
      ATermAppl CommExpr = gsMakeCommExpr(gsMakeMultActName(ATmakeList2((ATerm)gsString2ATermAppl(CurrentId), (ATerm)gsString2ATermAppl(strcat(Underscore, CurrentId)))), gsString2ATermAppl(strcat(Underscores, CurrentId)));
      ReturnList = ATinsert(ReturnList, (ATerm)CommExpr);
      List1 = ATgetNext(List1);
    }
    return ReturnList;
  }

  //==================================================
  // pn2gsPlacesParameters generates a list of all the parameters of the places
  //==================================================
  static ATermList pn2gsPlacesParameters(ATermList Places) {
    // input: a list containg all the places
    // output: a list in which all the elements of the inputed list
    //         are preceded by "pm_"

    ATermList ReturnList = ATmakeList0();
    while (ATisEmpty(Places) == ATfalse) {
      ReturnList = ATinsert(ReturnList, (ATerm)pn2gsPlaceParameterNat(ATgetName(ATgetAFun(ATgetFirst(Places)))));
      Places = ATgetNext(Places);
    }
    return ReturnList;
  }

  //==================================================
  // pn2gsMerge creates the GenSpect merge of all elements of the list, with a parameter!
  //==================================================
  static ATermAppl pn2gsMerge(ATermList Ids) {
    // input: a not-empty list with the ID's
    // output: an AtermAppl that is the MERGE of all the ID's in the input-list, each with its parameter!

    ATermAppl merge;
    char * CurrentElement = ATgetName(ATgetAFun(ATgetFirst(Ids)));
    if (ATgetLength(Ids) > 1) {
      merge = gsMakeMerge(pn2gsMerge(ATgetNext(Ids)), gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList1((ATerm)pn2gsPlaceParameterNat(CurrentElement))));
    } else {
      // ATgetLength(Ids) == 1, since the input-list is not-empty!
      merge = gsMakeActionProcess(ATmakeAppl0(ATmakeAFun(CurrentElement, 0, ATtrue)), ATmakeList1((ATerm)pn2gsPlaceParameterNat(CurrentElement)));
    }
    return merge;
  }

  //==================================================
  // pn2gsInitialMarkings creates a list with all the initial markings
  //==================================================
  static ATermList pn2gsInitialMarkings(ATermList Places, ATermTable Markings) {
    // input: a list with the places and a table with the markings
    // output: a list which contains all the initial markings of the places

    ATermList ReturnList = ATmakeList0();
    while (ATisEmpty(Places) == ATfalse) {
      ATerm MarkingUnQuoted = ATtableGet(Markings, ATgetFirst(Places));
      gsDebugMsg("Initial Marking unquoted = %t\n", MarkingUnQuoted);
      ATermAppl MarkingQuoted = ATmakeAppl0(ATmakeAFun(ATwriteToString(MarkingUnQuoted), 0, ATtrue));
      gsDebugMsg("Initial Marking quoted= %t\n", MarkingQuoted);
      ReturnList = ATinsert(ReturnList, (ATerm)gsMakeNumber(MarkingQuoted, gsMakeSortIdNat()));
      Places = ATgetNext(Places);
    }
    return ATreverse(ReturnList);
  }

  //==================================================
  // pn2gsGenerateProcEqns generates all the GenSpect Process Equations
  //==================================================
  static ATermList pn2gsGenerateProcEqns(ATermAppl NetID){
    // input: access to the context and the ID of the Petri net
    // output: an ATermList of <ProcEqn>'s
    
    // #processes = 5x #places + 3x #transitions + ~3
    // the actual number of processes might be less than the number stated here. 
    // the possible processes will be stored in ProcessList
    ATermList ProcessList = ATmakeList0();
    
    //==================================================
    // first, we generate the transition processes.
    //==================================================
    
    // In the comment below, ti will refer to 'transid' or to 'transid_transname'.
    // This depends on whether or not a transition has a name, and if it does, if it's not too long.
    
    // Each transition creates the following processes:
    // T_ti     = T_ti_in | T_ti_out | t_ti_mon;
    // T_ti_in  = "synchronisation of the incoming arcs"
    // T_ti_out = "synchronisation of the outgoing arcs"
    
    // If there are no incoming or outgoing arcs, T_ti_in respectively T_ti_out are left out!

    gsDebugMsg("\n\nStart creation of processes belonging to transitions.\n\n");
    for(ATermList Ids = ATtableKeys(context.trans_name);!ATisEmpty(Ids);Ids = ATgetNext(Ids))
      ProcessList = ATconcat(pn2gsGenerateTransition(ATgetFirst(Ids)),ProcessList);

    //==================================================
    // second, we generate the place processes.
    //==================================================

    // In the comment below, pi will refer to 'placeid' or to 'placeid_placename'.
    // This depends on whether or not a place has a name, and if it does, if it's not too long.
    
    // Each place creates the following processes:

    // the values n and m indicate the maximum concurrency of the place
    // P_pi(X:Nat)     =  sum y:Pos. (y<=n) -> P_pi_add(y) . P_pi(X+y)
    //                  + sum z:Pos. (z<=min(m,X)) -> P_pi_rem(z) . P_pi(max(0,X-z))
    //                  + sum y,z:Pos. (y<=n && z<=min(m,X)) -> (P_pi_add(y) | P_pi_rem(z)) . P_pi(max(0,X+y-z))
    //
    // P_pi_add(y:Pos) = restrict({"set of synchronisation of each incoming arc with itself"}, (y>1) -> (P_pi_in | P_pi_add(max(1, y-1))) : P_pi_in)
    //
    // P_pi_in = "choice of the incoming arcs"
    //
    // P_pi_rem(z:Pos) = restrict({"set of synchronisation of each outgoing arc with itself"}, (z>1) -> (P_pi_out | P_pi_rem(max(1, z-1))) : P_pi_out)
    //
    // P_pi_out = "choice of the outgoing arcs" 

    // If there are no incoming or outgoing arcs, P_pi_add and P_pi_in respectively P_pi_add and P_pi_out are will never be executed
    // because of the fact that the maximum concurrency == 0. Since 'y' and 'z' are of type Pos, they must be > 0 and thus execution
    // of the named processes is prevented.
    // Therefore, in that case it doesn't matter what P_pi_add and P_pi_in respectively P_pi_add and P_pi_out look like.
    // For ease of implementation, the parts in between " " will be defined as tau.

    gsDebugMsg("\n\nStart creation of processes belonging to places.\n\n");
    for(ATermList Ids = ATtableKeys(context.place_name);!ATisEmpty(Ids);Ids = ATgetNext(Ids)) 
      ProcessList = ATconcat(pn2gsGeneratePlace(ATgetFirst(Ids)),ProcessList);

    //==================================================
    // third, we generate the three general Petri net processes.
    //==================================================

    gsDebugMsg("\n\nStart creation of general processes.\n\n");
    // Trans =  "..." . Trans
    // "..." is the choice of all transitions. This guarantees interleaving.

    // If no interleaving is desired, instead of a choice of all transitions,
    // this should be the parallelisation of all transitions.
    // Also, the transition-processes should be extended with: " . T_ti "
    if (ATisEmpty(context.transitions) == ATtrue) {
      // there are no transitions
      ProcessList = ATinsert(ProcessList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(gsString2ATermAppl("Trans"), ATmakeList0()), ATmakeList0(), gsMakeDelta()));
    } else {
      // there are transitions
      ProcessList = ATinsert(ProcessList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(gsString2ATermAppl("Trans"), ATmakeList0()), ATmakeList0(), gsMakeSeq(pn2gsChoiceIn(context.transitions), gsMakeProcVarId(gsString2ATermAppl("Trans"), ATmakeList0()))));
    }
    gsDebugMsg("Process Trans created.\n");
    
    // PetriNet("...") = hide(I, restrict(H, comm(C, ("..." || Trans))));
    // the first "..." are the parameters of PetriNet; one for every place
    // the second "..." is the parallelisation of all the places in the PetriNet
    ATermList ParameterList;
    ATermAppl Process;
      
    ParameterList = pn2gsPlacesParameters(context.places);
    Process = gsMakeHide(pn2gsHideList(ATtableKeys(context.arc_in), ATtableKeys(context.arc_out)), gsMakeRestrict(pn2gsRestrictList(ATtableKeys(context.arc_in), ATtableKeys(context.arc_out)), gsMakeComm(pn2gsCommList(ATtableKeys(context.arc_in), ATtableKeys(context.arc_out)), gsMakeMerge(pn2gsMerge(context.places), gsMakeProcVarId(gsString2ATermAppl("Trans"), ATmakeList0())))));

    ProcessList = ATinsert(ProcessList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(gsString2ATermAppl("PetriNet"), ATmakeList0()), ParameterList, Process));
    gsDebugMsg("Process PetriNet created.\n");

    // reminder: NetID == "Net_'ID of the Petri net'"
    // the Net_ is preceded to the ID of the Petri net to prevent that two processes would have the same name!
    //
    // NetID = PetriNet("...")
    // "..." is the initial marking of all places

    Process = gsMakeActionProcess(gsString2ATermAppl("PetriNet"), pn2gsInitialMarkings(ATtableKeys(context.place_mark), context.place_mark));
    ProcessList = ATinsert(ProcessList, (ATerm)gsMakeProcEqn(ATmakeList0(), gsMakeProcVarId(NetID, ATmakeList0()), ATmakeList0(), Process));
    gsDebugMsg("Process %t created.\n", NetID);

    //Now reverse the whole thing
    return ATreverse(ProcessList);
  }

  /*                           */
  /*                           */
  /*                           */
  /* Generation-phase function */
  /*                           */
  /*                           */
  /*                           */

  //==================================================
  // pn2gsTranslate converts the ATerm delivered by pn2gsAterm to a GenSpect ATerm.
  //==================================================
  static ATermAppl pn2gsTranslate(ATermAppl Spec){
    // input: an ATermAppl that contains the translated PNML-file
    // output: another ATermAppl, which is the GenSpect translation
    //         of the PNML-ATerm.
    
    //==================================================
    // initializations of tables and some initial checks.
    //==================================================
    
    gsDebugMsg("\n====================\n\nStart generating the necessary data. \n \n");
    
    // put the places, transitions and arcs in the lists again
    ATermList APlaces = (ATermList)ATgetArgument(Spec, 0);
    ATermList ATransitions = (ATermList)ATgetArgument(Spec, 1);
    ATermList AArcs = (ATermList)ATgetArgument(Spec, 2);
    
    // temporary variable to store the current key
    // used for Places, Transitions and Arcs!!!
    ATerm CurrentKey;
    
    gsDebugMsg("> Insert the data of places that will be translated into tables...  \n");
    while (ATisEmpty(APlaces) == ATfalse) {
      // this loop itterates all places that will be translated
      gsDebugMsg("    examining %t\n", ATgetFirst(APlaces));
      CurrentKey = ATgetArgument(ATgetFirst(APlaces), 0);
      // it is sufficient to check whether a key appears in place_name OR place_mark
      // since a value is inserted in both tables at the same time!
      if (ATtableGet(context.place_name, CurrentKey)) {
	// the ID of the current places appears more than once in the places.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else {
	// insert the data into context.place_name
	// key = id
	// value = name
	ATtablePut(context.place_name, CurrentKey , ATgetArgument(ATgetFirst(APlaces), 1));
	
	// insert the data into context.place_mark
	// key = id
	// value = initialMarking
	ATtablePut(context.place_mark, CurrentKey , ATgetArgument(ATgetFirst(APlaces), 2));
      }
      
      // remove the entry from the list
      APlaces = ATgetNext(APlaces);
    }
    gsDebugMsg("\n  Places that are not inserted into the tables: %t\n", (ATerm)APlaces);
    gsDebugMsg("  ID's of the two read-in place tables.\n  THESE TWO LISTS SHOULD BE EXACTLY THE SAME!\n  ");
    gsDebugMsg("%t \n  %t \n", (ATerm)ATtableKeys(context.place_name), (ATerm)ATtableKeys(context.place_mark));
    
    gsDebugMsg("> Insert the the data of transitions that will be translated into tables...  \n");
    while (ATisEmpty(ATransitions) == ATfalse) {
      // this loop itterates all transitions that will be translated
      gsDebugMsg("    examining %t\n", ATgetFirst(ATransitions));
      CurrentKey = ATgetArgument(ATgetFirst(ATransitions), 0);
      if (ATtableGet(context.place_name, CurrentKey)) {
	// the ID of the current transition appeared already in the places.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else if (ATtableGet(context.trans_name, CurrentKey)) {
	// the ID of the current transition appears more than once in the transitions.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else {
	// insert the data into context.trans_name
	// key = id
	// value = name
	ATtablePut(context.trans_name, CurrentKey , ATgetArgument(ATgetFirst(ATransitions), 1));
      }
      // remove the entry from the list ATransitions
      ATransitions = ATgetNext(ATransitions);
    }
    gsDebugMsg("\n  Transitions that are not inserted into the tables: %t\n", (ATerm)ATransitions);
    gsDebugMsg("  ID's of the read-in transition table.\n  %t\n", (ATerm)ATtableKeys(context.trans_name));
    
    // temporary variables to store the current source and target
    ATerm CurrentSource;
    ATerm CurrentTarget;
    gsDebugMsg("> Insert the data of the arcs that will be translated into tables...  \n");
    while (ATisEmpty(AArcs) == ATfalse) {
      // this loop itterates all arcs that will be translated
      gsDebugMsg("    examining %t\n", ATgetFirst(AArcs));
      CurrentKey = ATgetArgument(ATgetFirst(AArcs), 0);
      if (ATtableGet(context.place_name, CurrentKey)) {
	// the ID of the current arc appeared already in the places.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else if (ATtableGet(context.trans_name, CurrentKey)) {
	// the ID of the current arc appeared already in the transitions.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else if (ATtableGet(context.arc_in, CurrentKey)) {
	// the ID of the current arc appeared already in the in_arcs.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else if (ATtableGet(context.arc_out, CurrentKey)) {
	// the ID of the current arc appeared already in the out_arcs.
	// this is an error in the input, and thus termination takes place!
	gsErrorMsg("The id: '%t' appears more than once!\n", CurrentKey);
	return NULL;
      } else {
	// the arc's ID did not appear in the transitions, places or arcs that will be translated
	// check the source and the target from the arc to see if the arc is used in the translation!
	CurrentSource = ATgetArgument(ATgetFirst(AArcs),1);
	CurrentTarget = ATgetArgument(ATgetFirst(AArcs),2);
	if (ATtableGet(context.place_name, CurrentSource) && ATtableGet(context.trans_name, CurrentTarget)) {
	  // The arc is an arc_out; it goes from a place to a transition
	  // insert the data into context.arc_out
	  // key = id
	  // value = arc_out(source, target)
	  ATtablePut(context.arc_out, CurrentKey, (ATerm)ATmakeAppl2(ATmakeAFun("arc_out",2,ATfalse),CurrentSource,CurrentTarget));
	} else if (ATtableGet(context.place_name, CurrentTarget) && ATtableGet(context.trans_name, CurrentSource)) {
	  // The arc is an arc_in; it goes from a transition to a place
	  // insert the data into context.arc_in
	  // key = id
	  // value = arc_in(source, target)
	  ATtablePut(context.arc_in, CurrentKey, (ATerm)ATmakeAppl2(ATmakeAFun("arc_in",2,ATfalse),CurrentSource,CurrentTarget));
	} else {
	  // either the source or the target (or both) of the arc will not be translated
	  // therefore the arc will not be translated either!
	  gsWarningMsg("The source or target of arc with id '%t' will not be translated, and thus this arc will not be translated either.\n", CurrentKey);
	}
      }
      // remove the entry from the list ATransitions
      AArcs = ATgetNext(AArcs);
    }
    gsDebugMsg("\n  Arcs that are not inserted into the tables: %t\n", (ATerm)AArcs);
    
    gsDebugMsg("  ID's of the two read-in arc tables. The first list contains the in-arcs, the second one the out-arcs. \n");
    gsDebugMsg("  NO TWO ID'S SHOULD BE IN BOTH TABLES!\n");
    gsDebugMsg("%t\n  %t\n\n", (ATerm)ATtableKeys(context.arc_in), (ATerm)ATtableKeys(context.arc_out));
    
    //==================================================
    // generation of some necessary data
    //==================================================
    
    // Temporary variables used for generations
    ATermList Arcs;
    ATerm CurrentArc;
    ATerm CurrentPlace;
    ATerm CurrentTrans;
    ATermList ArcValueList;
    
    // Generate context.place_in - context.trans_out
    Arcs = ATtableKeys(context.arc_in);
    while (ATisEmpty(Arcs) == ATfalse) {
      CurrentArc = ATgetFirst(Arcs);
      CurrentPlace = ATgetArgument(ATtableGet(context.arc_in, CurrentArc), 1);
      CurrentTrans = ATgetArgument(ATtableGet(context.arc_in, CurrentArc), 0);
      // insert CurrentPlace and CurrentArc in context.place_in
      if (!(ATtableGet(context.place_in, CurrentPlace))) {
	// if the CurrentPlace was not yet present in context.place_in, insert it
	// key = CurrentPlace.id
	// value = [CurrentArc.id]
	ATtablePut(context.place_in, CurrentPlace, (ATerm)ATmakeList1(CurrentArc));
      } else {
	// if the CurrentPlace was already present in context.place_in, insert CurrentArc.id in the value-list
	ArcValueList = (ATermList)ATtableGet(context.place_in, CurrentPlace);
	ArcValueList = ATinsert(ArcValueList, CurrentArc);
	ATtablePut(context.place_in, CurrentPlace, (ATerm)ArcValueList);
      }
      
      // insert CurrentTrans and CurrentArc in context.trans_out
      if (!(ATtableGet(context.trans_out, CurrentTrans))) {
	// if the CurrentTrans was not yet present in context.trans_out, insert it
	// key = CurrentTrans.id
	// value = [CurrentArc.id]
	ATtablePut(context.trans_out, CurrentTrans, (ATerm)ATmakeList1(CurrentArc));
      } else {
	// if the CurrentTrans was already present in context.trans_out, insert CurrentArc.id in the value-list
	ArcValueList = (ATermList)ATtableGet(context.trans_out, CurrentTrans);
	ArcValueList = ATinsert(ArcValueList, CurrentArc);
	ATtablePut(context.trans_out, CurrentTrans, (ATerm)ArcValueList);
      }
      Arcs = ATgetNext(Arcs);
    }
    Arcs = ATtableKeys(context.place_in);
    gsDebugMsg("context.place_in contains the following keys: %t\n", Arcs);
    while (ATisEmpty(Arcs) == ATfalse) {
      gsDebugMsg("Place '%t' has the following incoming arcs: %t\n", ATgetFirst(Arcs), ATtableGet(context.place_in, ATgetFirst(Arcs)));
      Arcs = ATgetNext(Arcs);
    }
    Arcs = ATtableKeys(context.trans_out);
    gsDebugMsg("context.trans_out contains the following keys: %t\n", Arcs);
    while (ATisEmpty(Arcs) == ATfalse) {
      gsDebugMsg("Transition '%t' has the following outgoing arcs: %t\n", ATgetFirst(Arcs), ATtableGet(context.trans_out, ATgetFirst(Arcs)));
      Arcs = ATgetNext(Arcs);
    }
    
    // Generate context.trans_in - context.place_out
    Arcs = ATtableKeys(context.arc_out);
    while (ATisEmpty(Arcs) == ATfalse) {
      CurrentArc = ATgetFirst(Arcs);
      CurrentPlace = ATgetArgument(ATtableGet(context.arc_out, CurrentArc), 0);
      CurrentTrans = ATgetArgument(ATtableGet(context.arc_out, CurrentArc), 1);
      // insert CurrentPlace and CurrentArc in context.place_out
      if (!(ATtableGet(context.place_out, CurrentPlace))) {
	// if the CurrentPlace was not yet present in context.place_out, insert it
	// key = CurrentPlace.id
	// value = [CurrentArc.id]
	ATtablePut(context.place_out, CurrentPlace, (ATerm)ATmakeList1(CurrentArc));
      } else {
	// if the CurrentPlace was already present in context.place_out, insert CurrentArc.id in the value-list
	ArcValueList = (ATermList)ATtableGet(context.place_out, CurrentPlace);
	ArcValueList = ATinsert(ArcValueList, CurrentArc);
	ATtablePut(context.place_out, CurrentPlace, (ATerm)ArcValueList);
      }
      
      // insert CurrentTrans and CurrentArc in context.trans_in
      if (!(ATtableGet(context.trans_in, CurrentTrans))) {
	// if the CurrentTrans was not yet present in context.trans_in, insert it
	// key = CurrentTrans.id
	// value = [CurrentArc.id]
	ATtablePut(context.trans_in, CurrentTrans, (ATerm)ATmakeList1(CurrentArc));
      } else {
	// if the CurrentTrans was already present in context.trans_in, insert CurrentArc.id in the value-list
	ArcValueList = (ATermList)ATtableGet(context.trans_in, CurrentTrans);
	ArcValueList = ATinsert(ArcValueList, CurrentArc);
	ATtablePut(context.trans_in, CurrentTrans, (ATerm)ArcValueList);
      }
      Arcs = ATgetNext(Arcs);
    }
    Arcs = ATtableKeys(context.place_out);
    gsDebugMsg("context.place_out contains the following keys: %t\n", Arcs);
    while (ATisEmpty(Arcs) == ATfalse) {
      gsDebugMsg("Place '%t' has the following outgoing arcs: %t\n", ATgetFirst(Arcs), ATtableGet(context.place_out, ATgetFirst(Arcs)));
      Arcs = ATgetNext(Arcs);
    }
    Arcs = ATtableKeys(context.trans_in);
    gsDebugMsg("context.trans_in contains the following keys: %t\n", Arcs);
    while (ATisEmpty(Arcs) == ATfalse) {
      gsDebugMsg("Transition '%t' has the following incoming arcs: %t\n", ATgetFirst(Arcs), ATtableGet(context.trans_in, ATgetFirst(Arcs)));
      Arcs = ATgetNext(Arcs);
    }
    
    gsDebugMsg("\n====================\n\n");
    
    //==================================================
    // creation of GenSpect ATerms
    //==================================================

    ATermList Actions;       // store all the GenSpect Actions
    ATermList ProcEqns;      // store all the GenSpect Process Equations
    
    Actions = pn2gsGenerateActions();
    ProcEqns = pn2gsGenerateProcEqns(ATAgetArgument(Spec, 3));
    
    gsDebugMsg("\n\n====================\n\n");
    gsDebugMsg("Conversion Succesful!");
    gsDebugMsg("\n\n====================\n\n");

    return gsMakeSpecV1(gsMakeSortSpec(ATmakeList0()), gsMakeConsSpec(ATmakeList0()), gsMakeMapSpec(ATmakeList0()), gsMakeDataEqnSpec(ATmakeList0()), gsMakeActSpec(Actions), gsMakeProcEqnSpec(ProcEqns), gsMakeInit(ATmakeList0(),gsMakeProcess(gsMakeProcVarId(ATAgetArgument(Spec, 3), ATmakeList0()), ATmakeList0())));
  }

  /*               */
  /*               */
  /*               */
  /* Main function */
  /*               */
  /*               */
  /*               */

  
  //==================================================
  // main
  //==================================================
  int main(int argc, char **argv){
    ATerm stackbot;
    ATinit(0,NULL,&stackbot);
    
#define sopts "ai"
    struct option lopts[] = {
      {"read-aterm"  , no_argument,      NULL, 'a'},
      {"debug"       , no_argument,      NULL, 'd'},
      {0, 0, 0, 0}
    };
    int opt,read_aterm;
    
    read_aterm = 0;
    while ( (opt = getopt_long(argc,argv,sopts,lopts,NULL)) != -1 ){
      switch ( opt ){
      case 'a':
	read_aterm = 1;
	break;
      case 'd': 
	gsSetDebugMsg();
	break;
      default:
	break;
      }
    }
    
    char *SpecStream;
    if ( argc-optind < 1 ){
      SpecStream = "stdin";
    } else {
      if ( (SpecStream = argv[optind]) == NULL ){
	perror(NAME);
	return 1;
      }
    }
    
    xmlDocPtr doc = xmlParseFile(SpecStream);
    if(!doc) {
      gsErrorMsg("Document not parsed succesfully. \n");
      return 1;
    }

    gsEnableConstructorFunctions();
    
    ATermAppl Spec=pn2gsAterm(doc);
    xmlFreeDoc(doc);
    
    if(!Spec){	
      gsErrorMsg("Error while converting PNML to ATerm, conversion stopped!  \n");
      return 1;
    }
    
    context.place_name=ATtableCreate(63,50);
    context.place_mark=ATtableCreate(63,50); 
    context.trans_name=ATtableCreate(63,50); 
    context.arc_in=ATtableCreate(63,50);  
    context.arc_out=ATtableCreate(63,50);    
    
    context.place_in=ATtableCreate(63,50);   
    context.trans_in=ATtableCreate(63,50);   
    context.place_out=ATtableCreate(63,50);  
    context.trans_out=ATtableCreate(63,50);  

    context.transitions=ATmakeList0();
    context.places=ATmakeList0();
    
    Spec=pn2gsTranslate(Spec);
    
    ATtableDestroy(context.place_name);
    ATtableDestroy(context.place_mark); 
    ATtableDestroy(context.trans_name); 
    ATtableDestroy(context.arc_in);  
    ATtableDestroy(context.arc_out);    
    
    ATtableDestroy(context.place_in);   
    ATtableDestroy(context.trans_in);   
    ATtableDestroy(context.place_out);  
    ATtableDestroy(context.trans_out);
    
    if(!Spec) {
      gsErrorMsg("Error while converting PNML ATerm to GenSpect ATerm, conversion stopped!  \n");
      return 1;
    }
    gsPrintSpecification(stdout,Spec);        
    return 0;
  }


  // Added by Yarick: alternative generation of Places:
  static ATermList pn2gsGeneratePlaceAlternative(ATerm PlaceID);
  static ATermAppl pn2gsGenerateP_pi_ar(int in, int out, ATermList In, ATermList Out);
  static ATermList pn2gsGetActionLists(int n, ATermList ActList);

  //==================================================
  // pn2gsGeneratePlaceAlternative generates the GenSpect Process Equations belonging to one place
  // without recursive parallelism
  //==================================================
  static ATermList pn2gsGeneratePlaceAlternative(ATerm PlaceID){
    // input: access to the context; ID of the place
    // output: an updated list of the processes: the processes belonging to the place added to the list

    //==================================================
    // retrieve the maximum concurrency
    //==================================================
    int n=0;
    ATermList ActsIn=ATLtableGet(context.place_in, PlaceID);
    if (ActsIn) n=ATgetLength(ActsIn);

    int m=0;
    ATermList ActsOut=ATLtableGet(context.place_out, PlaceID);
    if (ActsOut) m=ATgetLength(ActsOut);

    gsDebugMsg("Place %t has maximum concurrency in: '%d' and out: '%d'\n", PlaceID, n,m);

    //==================================================
    // generate the processes
    //==================================================

    //==================================================
    // second, we generate the place processes.
    //==================================================
    
    // In the comment below, pi will refer to 'placeid' or to 'placeid_placename'.
    // This depends on whether or not a place has a name, and if it does, if it's not too long.
    
    // Each place creates the following processes:
    
    // the values n and m indicate the maximum in/out concurrency of the place
    // we can add 0 to n tokens and remove 0 to min(m,X) token at one time.
    // we don't know X at generation time, so we have to generate for 0 to m possibilities.
    //  
    // P_pi(x:Nat)     = 
    //                    0<=x ->(0,0 case we skip
    //                   (skip)  +P_pi_ar_1_0.P_pi(max(x+1-0,0)) we skip max and -0.
    //                           +P_pi_ar_2_0.P_pi(max(x+2-0,0))
    //                           ...
    //                           +P_pi_ar_n_0.P_pi(max(x+n-0,0))
    //                           )
    //                   +1<=x ->(P_pi_ar_0_1.P_pi(max(x+0-1,0))
    //                           +P_pi_ar_1_1.P_pi(max(x+1-1,0))
    //                           +P_pi_ar_2_1.P_pi(max(x+2-1,0))
    //                           ...
    //                           +P_pi_ar_n_1.P_pi(max(x+n-1,0))
    //                           )
    //                   +
    //                   ....
    //                   +m<=x ->(P_pi_ar_0_m.P_pi(max(x+0-m,0))
    //                           +P_pi_ar_1_m.P_pi(max(x+1-m,0))
    //                           +P_pi_ar_2_m.P_pi(max(x+2-m,0))
    //                           ...
    //                           +P_pi_ar_n_m.P_pi(max(x+n-m,0))
    //                           )
    //
    // P_pi_ar_i_j = all possible multiactions with i incoming and j outgoing arcs.
    // need to be generated for all 0=<i<=n,0<=j<=m,i+j>0.
    // variables to store the name and id of the place

    //Calculate the name of the process 
    AFun CurrentPlaceId = ATgetAFun(PlaceID);

    {
      char * CurrentPlaceName = ATgetName(ATgetAFun(ATtableGet(context.place_name, PlaceID)));
      
      if(strcmp(CurrentPlaceName, "default_name"))
	// name of the place is not "default_name"
	// name of the place may be used
	CurrentPlaceId=ATappendAFun(ATappendAFun(CurrentPlaceId,"_"),CurrentPlaceName);

      //Prepend "P_"
      CurrentPlaceId=ATprependAFun("P_",CurrentPlaceId);
    }
    // variables to store the process names
    ATermAppl CurrentPlace=ATmakeAppl0(CurrentPlaceId);
    AFun CurrentPlaceARId=ATappendAFun(CurrentPlaceId,"_ar");
    //ATermAppl CurrentPlaceAdd=ATmakeAppl0(ATappendAFun(CurrentPlaceId,"_add"));
    //ATermAppl CurrentPlaceIn=ATmakeAppl0(ATappendAFun(CurrentPlaceId,"_in"));
    //ATermAppl CurrentPlaceRem=ATmakeAppl0(ATappendAFun(CurrentPlaceId,"_rem"));
    //ATermAppl CurrentPlaceOut=ATmakeAppl0(ATappendAFun(CurrentPlaceId,"_out"));
    
    // insert the name of the process P_pi into context.places
    // this is needed for the generation of the general process PetriNet
    context.places = ATinsert(context.places, (ATerm)CurrentPlace);
    gsDebugMsg("context.places now contains the following places: %t\n", context.places);
    
    
    ATermList EquationList=ATmakeList0();
    {
      //generate the main process
      ATermAppl VarX=ATmakeAppl0(ATmakeAFunId("x"));
      AFun CurrentPlaceARId=ATappendAFun(CurrentPlaceId,"_ar");
      ATermAppl Body=NULL;
      for(int j=m;j>-1;j--){
	ATermAppl Summand=NULL;
	for(int i=n;i>-1;i--){
	  if(j==0 && i==0) continue;
	  ATermAppl Left=ATermAppl0(CurrentPlaceARId);//make name P_pi_ar_i_j
	  ATermAppl Right=0;//make P_pi(max(X+i-j,0))
	  ATermAppl Sec=gsMakeSeq(Left,Right);
	  if(Summand) Summand=gsMakeChoice(Sec,Summand);
	  else Summand=Sec; 

	  //generate the additional process
	  EquationList = ATinsert(EquationList, 
				  (ATerm)gsMakeProcEqn(ATmakeList0(), 
						       gsMakeProcVarId(Left, ATmakeList0()), 
						       ATmakeList0(), 
						       pn2gsGenerateP_pi_ar(i,j,ActsIn,ActsOut)));
	}
	
	if(j>0){ //generate the condition
	  ATermAppl Cond=gsMakeDataExprLTE(gsMakeDataExprPos2Nat(ATmakeAppl0(ATmakeAFunInt0(j))),VarX);//make j<=x
	  Summand=gsMakeCond(Cond,Summand,gsMakeDelta());
	}
	
	if(Body) Body=gsMakeChoice(Summand,Body);
	else Body=Summand;
      }

      // handle the case m+n=0.
      if(!Body) Body=gsMakeDelta();

      //make process P_pi and add it
      EquationList = ATinsert(EquationList, 
			      (ATerm)gsMakeProcEqn(ATmakeList0(), 
						   gsMakeProcVarId(CurrentPlace, 
								   ATmakeList1((ATerm)gsMakeSortIdNat())), 
						   ATmakeList1((ATerm)gsMakeDataVarId(VarX,
										      gsMakeSortIdNat())), 
						   Body));
    }
    
  }

  static ATermAppl pn2gsGenerateP_pi_ar(int in, int out, ATermList In, ATermList Out){
    //input: the exact numbers of in and out actions, the sets of these actions (as lists).
    //output: a process that is the choice of all multiactions (order not important)

    ATermList ResList=ATmakeList0();
    {
      ATermList InActionLists=pn2gsGetActionLists(in,In);
      ATermList OutActionLists=pn2gsGetActionLists(out,Out);
      //pairwise merge the elements of the 2 lists into 1 big list. 
      for(;!ATisEmpty(InActionLists);InActionLists=ATgetNext(InActionLists)){
	ATermList CurInActList=ATLgetFirst(InActionLists);
	for(;!ATisEmpty(OutActionLists);OutActionLists=ATgetNext(OutActionLists)){
	  ATermList CurOutActList=ATLgetFirst(InActionLists);
	  ResList=ATinsert(ResList,(ATerm)ATconcat(CurInActList,CurOutActList));
	}
      }
      //ResList=ATreverse(ResList);
    }
    //make a sum of multi-actions out of the merged list-of-lists and returm it
    
  }

  static ATermList pn2gsGetActionLists(int n, ATermList ActList){
    //returns all sublists (not necessarily consecutive) of length n
    
    //cannot
    if(n>ATgetLength(ActList)) return ATmakeList0();
    
    //one way
    if(n==ATgetLength(ActList)) return ATmakeList1((ATerm)ActList);
    
    //the only sublist of length 0 is []
    if(!n) return ATmakeList1((ATerm)ATmakeList0());

    //0<n<len(list); therefore len(list) > 0: 2 possibilities: 
    //- take the first element, get the recursive possibilities with n-1, combine.
    ATermAppl FirstAction=ATAgetFirst(ActList);
    ActList=ATgetNext(ActList);

    ATermList RestN1=pn2gsGetActionLists(n-1,ActList);

    //- not take the first element, get the recursive possibilities with n.
    ATermList RestN=pn2gsGetActionLists(n,ActList);
    
    //add the first Action to all the elements of RestN1
    {
      ATermList NewRestN1=ATmakeList0();
      for(;!ATisEmpty(RestN1);RestN1=ATgetNext(RestN1)){
	ATermList CurAList=ATLgetFirst(RestN1);
	CurAList=ATinsert(CurAList,(ATerm)FirstAction);
	NewRestN1=ATinsert(NewRestN1,(ATerm)CurAList);
      }
      RestN1=ATreverse(NewRestN1);
    }
    return ATconcat(RestN1,RestN);
  }

  
#ifdef __cplusplus
}
#endif
