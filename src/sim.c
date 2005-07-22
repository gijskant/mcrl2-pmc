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
#include "libgsparse.h"
#include "libgsnextstate.h"
#include "libgsrewrite.h"

static void gsPrintState(ATerm state)
{
	for (int i=0; i<gsGetStateLength(); i++)
	{
		if ( i > 0 )
		{
			ATprintf(", ");
		}

		ATermAppl a = gsGetStateArgument(state,i);
		if ( gsIsDataVarId(a) )
		{
			ATprintf("_");
		} else {
			gsPrintPart(stdout,a,false,0);
		}
	}
}

int main(int argc, char **argv)
{
	FILE *SpecStream;
	ATerm stackbot, state;
	ATermAppl Spec;
	ATermList states, l;
	#define sopts "d"
	struct option lopts[] = {
		{ "dummy",	no_argument,	NULL,	'd' },
		{ 0, 0, 0, 0 }
	};
	int opt, i, r;
	bool usedummy;

	ATinit(argc,argv,&stackbot);

	usedummy = false;
	while ( (opt = getopt_long(argc,argv,sopts,lopts,NULL)) != -1 )
	{
		switch ( opt )
		{
			case 'd':
				usedummy = true;
				break;
			default:
				break;
		}
	}

	if ( argc-optind < 1 )
	{
		fprintf(stderr,"Usage: %s <lpe file>\n",NAME);
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

	state = gsNextStateInit(Spec,!usedummy,GS_STATE_VECTOR,GS_REWR_INNER3);

	ATprintf("initial state: [ ");
	gsPrintState(state);
	ATprintf(" ]\n\n");

	while ( 1 )
	{
		states = gsNextState(state,NULL);
		if ( ATisEmpty(states) )
		{
			printf("deadlock\n\n");
		}
		for (l=states,i=0; !ATisEmpty(l); l=ATgetNext(l), i++)
		{
			ATprintf("%i: ",i);
			gsPrintPart(stdout,ATAgetFirst(ATLgetFirst(l)),false,0);
			ATprintf("  ->  [ ");
			gsPrintState(ATgetFirst(ATgetNext(ATLgetFirst(l))));
			ATprintf(" ]\n\n");
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
		ATprintf("\ntransition: ");
		gsPrintPart(stdout,ATAgetFirst(ATLelementAt(states,i)),false,0);
		ATprintf("\n\n");
		state = ATgetFirst(ATgetNext(ATLelementAt(states,i)));
		ATprintf("current state: [ ");
		gsPrintState(state);
		ATprintf(" ]\n\n");
	}
}

#ifdef __cplusplus
}
#endif
