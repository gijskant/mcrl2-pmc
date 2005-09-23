/* $Id: sim.c,v 1.1 2005/05/03 15:44:47 muck Exp $ */
#ifdef __cplusplus
extern "C" {
#endif

#define NAME "sim"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <aterm2.h>
#include "gslowlevel.h"
#include "gsfunc.h"
#include "libprint_c.h"
#include "libgsnextstate.h"
#include "libgsrewrite.h"

static void gsPrintState(ATerm state)
{
	for (int i=0; i<gsGetStateLength(); i++)
	{
		if ( i > 0 )
		{
			gsprintf(", ");
		}

		ATermAppl a = gsGetStateArgument(state,i);
		if ( gsIsDataVarId(a) )
		{
			gsprintf("_");
		} else {
			gsprintf("%P",a);
		}
	}
}

void print_help(FILE *f, char *Name)
{
	fprintf(f,"Usage: %s OPTIONS LPEFILE\n",Name);
	fprintf(f,"Simulates the LPE in LPEFILE.\n"
		  "(Enter '-1' or use CTRL-d to end simulation.)\n"
		  "\n"
	          "The OPTIONS that can be used are:\n"
	          "-h, --help               Display this help message\n"
	          "-y, --dummy              Replace free variables in the LPE\n"
		  "                         with dummy values\n"
	          "-R, --rewriter name      Use rewriter 'name' (default inner3)\n"
      );
}

int main(int argc, char **argv)
{
	FILE *SpecStream;
	ATerm stackbot, state;
	ATermAppl Spec;
	ATermList states, l;
	#define sopts "hyR:"
	struct option lopts[] = {
		{ "help",	no_argument,	NULL,	'h' },
		{ "dummy",	no_argument,	NULL,	'y' },
		{ "rewriter",	no_argument,	NULL,	'R' },
		{ 0, 0, 0, 0 }
	};
	int opt, i, r;
	RewriteStrategy strat;
	bool usedummy;

	ATinit(argc,argv,&stackbot);

	usedummy = false;
	strat = GS_REWR_INNER3;
	while ( (opt = getopt_long(argc,argv,sopts,lopts,NULL)) != -1 )
	{
		switch ( opt )
		{
			case 'h':
				print_help(stderr, argv[0]);
				return 0;
			case 'y':
				usedummy = true;
				break;
			case 'R':
				strat = RewriteStrategyFromString(optarg);
				if ( strat == GS_REWR_INVALID )
				{
					gsErrorMsg("invalid rewrite strategy '%s'\n",optarg);
					return 1;
				}
				break;
			default:
				break;
		}
	}

	if ( argc-optind < 1 )
	{
		print_help(stderr, argv[0]);
		return 1;
	}

	if ( (SpecStream = fopen(argv[optind],"r")) == NULL )
	{
		perror(NAME);
		return 1;
	}
	gsEnableConstructorFunctions();
	Spec = (ATermAppl) ATreadFromFile(SpecStream);
	if ( Spec == NULL )
	{
		return 1;
	}

	state = gsNextStateInit(Spec,!usedummy,GS_STATE_VECTOR,strat);

	gsprintf("initial state: [ ");
	gsPrintState(state);
	gsprintf(" ]\n\n");

	while ( 1 )
	{
		states = gsNextState(state,NULL);
		if ( ATisEmpty(states) )
		{
			printf("deadlock\n\n");
		}
		for (l=states,i=0; !ATisEmpty(l); l=ATgetNext(l), i++)
		{
			gsprintf("%i: %P  ->  [ ",i,ATAgetFirst(ATLgetFirst(l)));
			gsPrintState(ATgetFirst(ATgetNext(ATLgetFirst(l))));
			gsprintf(" ]\n\n");
		}
harm:
		printf("? "); fflush(stdout);
		r = 0;
		while ( r == 0 )
		{
			if ( (r = scanf("%i",&i)) == EOF )
			{
				r = i = -1;
			} else {
				fgetc(stdin);
			}
		}
		if ( i < 0 )
			break;
		if ( i >= ATgetLength(states) )
		{
			goto harm;
		}
		gsprintf("\ntransition: %P\n\n",ATAgetFirst(ATLelementAt(states,i)));
		state = ATgetFirst(ATgetNext(ATLelementAt(states,i)));
		gsprintf("current state: [ ");
		gsPrintState(state);
		gsprintf(" ]\n\n");
	}
}

#ifdef __cplusplus
}
#endif
