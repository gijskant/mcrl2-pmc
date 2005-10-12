#ifdef __cplusplus
extern "C" {
#endif

#define NAME "alpha"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <aterm2.h>
#include "gslowlevel.h"
#include "gsfunc.h"
#include "libprint_types.h"
#include "libprint_c.h"
#include "libgsalpha.h"

extern ATermAppl gsParse(FILE *);
extern ATermAppl gsTypeCheck(ATermAppl);

void print_help(FILE *f, char *Name)
{
	fprintf(f,"Usage: %s OPTIONS [SPECFILE [OUTFILE]]\n",Name);
	fprintf(f,"Apply alphabet axioms to processes in SPECFILE and save the result to OUTFILE.\n"
		  "If OUTFILE is not present, stdout is used. If SPECFILE is not present, stdin is\n"
		  "used. To use stdin and save the output into a file, use '-' for SPECFILE.\n"
	          "\n"
	          "The OPTIONS that can be used are:\n"
	          "-h, --help               display this help message\n"
	          "-a, --read-aterm         SPECFILE is an ATerm\n"
	          "-w, --write-aterm        OUTFILE should be an ATerm\n"
	       );
}

int main(int argc, char **argv)
{
	FILE *SpecStream, *OutStream;
	ATerm stackbot;
	ATermAppl Spec;
	#define sopts "haw"
	struct option lopts[] = {
		{ "help",		no_argument,	NULL,	'h' },
		{ "read-aterm",		no_argument,	NULL,	'a' },
		{ "write-aterm",	no_argument,	NULL,	'w' },
		{ 0, 0, 0, 0 }
	};
	int opt,read_aterm,write_aterm;

	ATinit(argc,argv,&stackbot);

	read_aterm = 0;
	write_aterm = 0;
	while ( (opt = getopt_long(argc,argv,sopts,lopts,NULL)) != -1 )
	{
		switch ( opt )
		{
			case 'h':
				print_help(stderr, argv[0]);
				return 0;
			case 'a':
				read_aterm = 1;
				break;
			case 'w':
				write_aterm = 1;
				break;
			default:
				break;
		}
	}

	SpecStream = stdin;
	if ( optind < argc && strcmp(argv[optind],"-") )
	{
		if ( (SpecStream = fopen(argv[optind],"r")) == NULL )
		{
			perror(NAME);
			return 1;
		}
	}

	OutStream = stdout;
	if ( optind+1 < argc )
	{
		if ( (OutStream = fopen(argv[optind+1],"w")) == NULL )
		{
			perror(NAME);
			return 1;
		}
	}

	if ( read_aterm )
	{
		gsEnableConstructorFunctions();
		Spec = (ATermAppl) ATreadFromFile(SpecStream);
	} else {
		gsEnableConstructorFunctions();
		Spec = gsParse(SpecStream);
		Spec = gsTypeCheck(Spec);
		fclose(SpecStream);
	}
	if ( Spec == NULL )
	{
		return 1;
	}

	Spec = Alpha(Spec);

	if ( write_aterm )
	{
		ATwriteToTextFile((ATerm) Spec,OutStream);
	} else {
		PrintPart_C(OutStream, (ATerm) Spec, ppAdvanced);
	}
	if ( OutStream != stdout )
	{
		fclose(OutStream);
	}
}

#ifdef __cplusplus
}
#endif
