#include <string>
#include <getopt.h>
#include "aterm2.h"
#include "liblowlevel.h"
#include "libstruct.h"
#include "libprint_c.h"
#include "liblts.h"
#include "liblts_fsm.h"

#define NAME "ltscp"
#define VERSION "0.1"
#include "mcrl2_revision.h"

using namespace std;
using namespace mcrl2::lts;

enum alt_lts_type { alt_lts_none, alt_lts_fsm };

static ATerm get_lpe(string &filename)
{
  if ( filename == "" )
  {
    return NULL;
  }

  FILE* file = fopen(filename.c_str(),"rb");
  if ( file == NULL )
  {
    gsErrorMsg("unable to open LPE-file '%s'\n",filename.c_str());
    return NULL;
  }
  
  ATerm t = ATreadFromFile(file);
  fclose(file);
  
  if ( (t == NULL) || (ATgetType(t) != AT_APPL) || !(gsIsSpecV1((ATermAppl) t) || !strcmp(ATgetName(ATgetAFun((ATermAppl) t)),"spec2gen")) )
  {
    gsErrorMsg("invalid LPE-file '%s'\n",filename.c_str());
    return NULL;
  }

  return t;
}

static lts_type get_extension(string s)
{
  string::size_type pos = s.find_last_of('.');
  
  if ( pos != string::npos )
  {
    string ext = s.substr(pos+1);

    if ( ext == "aut" )
    {
      gsVerboseMsg("detected AUT extension\n");
      return lts_aut;
    } else if ( ext == "svc" )
    {
      gsVerboseMsg("detected SVC extension; assuming mCRL2 format\n");
      return lts_mcrl2;
    }
  }

  return lts_none;
}

static alt_lts_type get_alt_extension(string s)
{
  string::size_type pos = s.find_last_of('.');
  
  if ( pos != string::npos )
  {
    string ext = s.substr(pos+1);

    if ( ext == "fsm" )
    {
      gsVerboseMsg("detected FSM extension\n");
      return alt_lts_fsm;
    }
  }

  return alt_lts_none;
}

static lts_type get_format(char *s)
{
  if ( !strcmp(s,"aut") )
  {
    return lts_aut;
  } else if ( !strcmp(s,"mcrl") )
  {
    return lts_mcrl;
  } else if ( !strcmp(s,"mcrl2") )
  {
    return lts_mcrl2;
  }

  return lts_none;
}

static alt_lts_type get_alt_format(char *s)
{
  if ( !strcmp(s,"fsm") )
  {
    return alt_lts_fsm;
  }

  return alt_lts_none;
}

static void print_formats(FILE *f)
{
  fprintf(f,
    "The following formats are accepted by " NAME ":\n"
    "\n"
    "  aut       the aldebaran format (.aut)\n"
    "  mcrl      the mCRL SVC format (.svc)\n"
    "  mcrl2     the mCRL2 SVC format (.svc, default)\n"
    "  fsm       the FSM format (.fsm, only output)\n"
    );
}

static void print_help(FILE *f, char *Name)
{
  fprintf(f,
    "Usage: %s [OPTION]... [INFILE [OUTFILE]]\n"
    "Copy the labelled transition system (LTS) in INFILE to OUTFILE, converting\n"
    "between formats if necessary (and possible). If OUTFILE is not supplied, the\n"
    "LTS is saved to stdout. If INFILE is not supplied, stdin is used instead.\n"
    "\n"
    "The format of INFILE and OUTFILE is determined by its contents or extension.\n"
    "Options --in and --out can be used to force the format for INFILE and OUTFILE,\n"
    "respectively.\n"
    "\n"
    "Mandatory arguments to long options are mandatory for short options too.\n"
    "  -h, --help            display this help message\n"
    "      --version         display version information\n"
    "  -q, --quiet           do not display warning messages\n"
    "  -v, --verbose         display concise intermediate messages\n"
    "  -i, --in=FORMAT       consider INFILE to be in the FORMAT format\n"
    "  -o, --out=FORMAT      save LTS in OUTFILE in the FORMAT format\n"
    "  -f, --formats         list accepted formats\n"
    "  -l, --lpe=FILE        supply LPE file from which the input file was generated\n"
    "                        (needed to store the correct parameter names of states\n"
    "                        when saving as FSM file)\n",
    Name);
}

static void print_version(FILE *f)
{
  fprintf(f,NAME " " VERSION " (revision %i)\n", REVISION);
}

int main(int argc, char **argv)
{
  ATerm bot;
  ATinit(argc,argv,&bot);
  gsEnableConstructorFunctions();


  #define ShortOptions      "hqvi:o:fl:"
  #define VersionOption     0x1
  struct option LongOptions[] = { 
    {"help"      , no_argument,         NULL, 'h'},
    {"version"   , no_argument,         NULL, VersionOption},
    {"quiet"     , no_argument,         NULL, 'q'},
    {"verbose"   , no_argument,         NULL, 'v'},
    {"in"        , required_argument,   NULL, 'i'},
    {"out"       , required_argument,   NULL, 'o'},
    {"formats"   , no_argument,         NULL, 'f'},
    {"lpe"       , required_argument,   NULL, 'l'},
    {0, 0, 0, 0}
  };

  bool verbose = false;
  bool quiet = false;
  lts_type intype = lts_none;
  lts_type outtype = lts_none;
  int opt;
  string lpefile;
  bool use_alt_outtype = false;
  alt_lts_type alt_outtype = alt_lts_none;
  while ( (opt = getopt_long(argc, argv, ShortOptions, LongOptions, NULL)) != -1 )
  {
    switch ( opt )
    {
      case 'h':
        print_help(stderr,argv[0]);
        return 0;
      case VersionOption:
        print_version(stderr);
        return 0;
      case 'v':
        verbose = true;
        break;
      case 'q':
        quiet = true;
        break;
      case 'i':
        if ( intype != lts_none )
        {
          fprintf(stderr,"warning: input format has already been specified; extra option ignored\n");
        } else {
          intype = get_format(optarg);
          if ( intype == lts_none )
          {
            fprintf(stderr,"warning: format '%s' is not recognised; option ignored\n",optarg);
          }
        }
        break;
      case 'o':
        if ( (outtype != lts_none) || use_alt_outtype )
        {
          fprintf(stderr,"warning: output format has already been specified; extra option ignored\n");
        } else {
          outtype = get_format(optarg);
          if ( outtype == lts_none )
          {
            alt_outtype = get_alt_format(optarg);
            if ( alt_outtype == alt_lts_none )
            {
              fprintf(stderr,"warning: format '%s' is not recognised; option ignored\n",optarg);
            } else {
              use_alt_outtype = true;
            }
          }
        }
        break;
      case 'f':
        print_formats(stderr);
        return 0;
      case 'l':
        if ( lpefile != "" )
        {
          fprintf(stderr,"warning: LPE file has already been specified; extra option ignored\n");
        }
        lpefile = optarg;
        break;
      default:
        break;
    }
  }

  if ( quiet && verbose )
  {
    gsErrorMsg("options -q/--quiet and -v/--verbose cannot be used together\n");
    return 1;
  }
  if ( quiet )
  {
    gsSetQuietMsg();
  }
  if ( verbose )
  {
    gsSetVerboseMsg();
  }


  bool use_stdin = (optind >= argc);
  bool use_stdout = (optind+1 >= argc);

  string infile;
  string outfile;
  if ( !use_stdin )
  {
    infile = argv[optind];
  }
  if ( !use_stdout )
  {
    outfile = argv[optind+1];
    if ( (outtype == lts_none) && !use_alt_outtype )
    {
      if ( lpefile != "" )
      {
        gsVerboseMsg("no output format set; using fsm because --lpe was used\n");
        alt_outtype = alt_lts_fsm;
        use_alt_outtype = true;
      } else {
        gsVerboseMsg("trying to detect output format by extension...\n");
        outtype = get_extension(outfile);
        if ( outtype == lts_none )
        {
          alt_outtype = get_alt_extension(outfile);
          if ( alt_outtype == alt_lts_none )
          {
            gsVerboseMsg("no output format set or detected; using default (mcrl2)\n");
            outtype = lts_mcrl2;
          } else {
            use_alt_outtype = true;
          }
        }
      }
    }
  } else {
      if ( lpefile != "" )
      {
        gsVerboseMsg("no output format set; using fsm because --lpe was used\n");
        alt_outtype = alt_lts_fsm;
        use_alt_outtype = true;
      } else {
        gsVerboseMsg("no output format set; using default (aut)\n");
        outtype = lts_aut;
      }
  }

  lts l;

  if ( use_stdin )
  {
    gsVerboseMsg("reading LTS from stdin...\n");
    if ( !l.read_from(cin,intype) )
    {
      gsErrorMsg("cannot read LTS from stdin\n");
      return 1;
    }
  } else {
    gsVerboseMsg("reading LTS from '%s'...\n",infile.c_str());
    if ( !l.read_from(infile,intype) )
    {
      bool b = true;
      if ( intype == lts_none ) // XXX really do this?
      {
        gsVerboseMsg("reading failed; trying to force format by extension...\n");
        intype = get_extension(infile);
        if ( (intype != lts_none) && l.read_from(infile,intype) )
        {
          b = false;
        }
      }
      if ( b )
      {
        gsErrorMsg("cannot read LTS from file '%s'\n",infile.c_str());
        return 1;
      }
    }
  }

  if ( use_stdout )
  {
    gsVerboseMsg("writing LTS to stdout...\n");
    if ( use_alt_outtype )
    {
      switch ( alt_outtype )
      {
        case alt_lts_fsm:
          if ( !write_lts_to_fsm(l,cout,get_lpe(lpefile)) )
          {
            gsErrorMsg("cannot write LTS to stdout\n");
            return 1;
          }
          break;
        default:
          assert(0);
          gsErrorMsg("do not know how to handle output format\n");
          return 1;
      }
    } else {
      if ( !l.write_to(cout,outtype) )
      {
        gsErrorMsg("cannot write LTS to stdout\n");
        return 1;
      }
    }
  } else {
    gsVerboseMsg("writing LTS to '%s'...\n",outfile.c_str());
    if ( use_alt_outtype )
    {
      switch ( alt_outtype )
      {
        case alt_lts_fsm:
          if ( !write_lts_to_fsm(l,outfile,get_lpe(lpefile)) )
          {
            gsErrorMsg("cannot write LTS to file '%s'\n",outfile.c_str());
            return 1;
          }
          break;
        default:
          assert(0);
          gsErrorMsg("do not know how to handle output format\n");
          return 1;
      }
    } else {
      if ( !l.write_to(outfile,outtype) )
      {
        gsErrorMsg("cannot write LTS to file '%s'\n",outfile.c_str());
        return 1;
      }
    }
  }

  return 0;
}
