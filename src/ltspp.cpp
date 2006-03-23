#include <iostream>
#include <fstream>
#include <getopt.h>
#include "liblowlevel.h"
#include "libstruct.h"
#include "libprint_c.h"
#include "libprint.h"
#include "liblts.h"
#include "setup.h"

#ifdef MCRL2_BCG
#include <bcg_user.h>
#endif

#define NAME     "ltspp"
#define VERSION  "0.1"
#include "mcrl2_revision.h"

#define ATisAppl(x) (ATgetType(x) == AT_APPL)
#define ATisList(x) (ATgetType(x) == AT_LIST)

using namespace std;
using namespace mcrl2::lts;
  
static bool write_lts_to_dot(lts &l, ostream &os, string &name, bool print_states)
{
  os << "digraph \"" << name << "\" {" << endl;
  // os << "size=\"7,10.5\";" << endl;
  os << "center=TRUE;" << endl;
  os << "mclimit=10.0;" << endl;
  os << "nodesep=0.05;" << endl;
  os << "node[width=0.25,height=0.25,label=\"\"];" << endl;
  if ( l.num_states() > 0 )
  {
    os << l.initial_state() << "[peripheries=2];" << endl;
    if ( print_states && l.has_state_info() )
    {
      for (unsigned int i=0; i<l.num_states(); i++)
      {
        ATerm state = l.state_value(i);
        if ( ATisAppl(state) ) // XXX better check for mCRL2
        {
          os << i << "[label=\"(";
          ATermList args = ATgetArguments((ATermAppl) state);
          for (; !ATisEmpty(args); args=ATgetNext(args))
          {
            PrintPart_CXX(os,ATgetFirst(args),ppDefault);
            if ( !ATisEmpty(ATgetNext(args)) )
              cout << ",";
          }
          os << ")\"];" << endl;
        } else if ( ATisList(state) )
        {
          os << i << "[label=\"[";
          ATermList args = (ATermList) state;
          for (; !ATisEmpty(args); args=ATgetNext(args))
          {
            os << ATwriteToString(ATgetFirst(args));
            if ( !ATisEmpty(ATgetNext(args)) )
              cout << ",";
          }
          os << "]\"];" << endl;
        } else {
          os << ATwriteToString(state);
        }
      }
    }
  }
  for (unsigned int i=0; i<l.num_transitions(); i++)
  {
    os << l.transition_from(i) << "->" << l.transition_to(i) << "[label=\"";
    if ( l.has_label_info() )
    {
      ATerm label = l.label_value(l.transition_label(i));
      if ( ATisAppl(label) )
      {
        ATermAppl t = (ATermAppl) label;
        if ( ATgetArity(ATgetAFun(t)) == 0 )
        {
          os << ATgetName(ATgetAFun(t));
        } else if ( gsIsMultAct(t) || is_timed_pair(t) )
        {
          if ( !gsIsMultAct(t) ) // for backwards compatibility with untimed svc version
          {
            t = ATAgetArgument(t,0);
          }
          if ( ATisEmpty(ATLgetArgument(t,0)) )
          {
            os << "tau";
          } else {
            PrintPart_CXX(os,(ATerm) t,ppDefault);
          }
        } else {
          os << ATwriteToString(label);
        }
      } else {
        os << ATwriteToString(label);
      }
    } else {
      os << l.transition_label(i);
    }
    os << "\"];" << endl;
  }

  os << "}" << endl;

  return true;
}

static string get_base(string &s)
{
  string::size_type pos = s.find_last_of('.');
  
  if ( pos == string::npos )
  {
    return s;
  } else {
    return s.substr(0,pos);
  }
}

static lts_type get_extension(string &s)
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
#ifdef MCRL2_BCG
    } else if ( ext == "bcg" )
    {
      gsVerboseMsg("detected BCG extension\n");
      return lts_bcg;
#endif
    }
  }

  return lts_none;
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
#ifdef MCRL2_BCG
  } else if ( !strcmp(s,"bcg") )
  {
    return lts_bcg;
#endif
  }

  return lts_none;
}

static void print_formats(FILE *f)
{
  fprintf(f,
    "The following formats are accepted by " NAME ":\n"
    "\n"
    "  aut       the aldebaran format (.aut)\n"
    "  mcrl      the mCRL SVC format (.svc)\n"
    "  mcrl2     the mCRL2 SVC format (.svc, default)\n"
#ifdef MCRL2_BCG
    "  bcg       the BCG format (.bcg)\n"
#endif
    );
}

static void print_help(FILE *f, char *Name)
{
  fprintf(f,
    "Usage: %s [OPTION]... [INFILE [OUTFILE]]\n"
    "Print the labelled transitions system (LTS) in INFILE to OUTFILE in the DOT format. If\n"
    "OUTFILE is not present, stdout is used. If INFILE is not present, stdin is used.\n"
    "\n"
    "The format of INFILE is determined by its contents or extension. The option --in\n"
    "can be used to force the format for INFILE.\n"
    "\n"
    "Mandatory arguments to long options are mandatory for short options too.\n"
    "  -h, --help            display this help and terminate\n"
    "      --version         display version information and terminate\n"
    "  -q, --quiet           do not display warning messages\n"
    "  -v, --verbose         display concise intermediate messages\n"
    "  -n, --no-state        do not save state information\n"
    "  -i, --in=FORMAT       consider INFILE to be in the FORMAT format\n"
    "  -f, --formats         list accepted formats\n",
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


  #define ShortOptions      "hqvni:f"
  #define VersionOption     0x1
  struct option LongOptions[] = { 
    {"help"      , no_argument,         NULL, 'h'},
    {"version"   , no_argument,         NULL, VersionOption},
    {"quiet"     , no_argument,         NULL, 'q'},
    {"verbose"   , no_argument,         NULL, 'v'},
    {"no-state"  , no_argument,         NULL, 'n'},
    {"in"        , required_argument,   NULL, 'i'},
    {"formats"   , no_argument,         NULL, 'f'},
    {0, 0, 0, 0}
  };

  bool verbose = false;
  bool quiet = false;
  bool print_state = true;
  lts_type intype = lts_none;
  int opt;
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
      case 'n':
        print_state = false;
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
      case 'f':
        print_formats(stderr);
        return 0;
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
    gsVerboseMsg("writing to stdout...\n");
    string name = "stdout";
    if ( !use_stdin )
    {
      name = get_base(infile);
    }
    write_lts_to_dot(l,cout,name,print_state);
  } else {
    gsVerboseMsg("writing to '%s'...\n",outfile.c_str());
    ofstream os(outfile.c_str(),ofstream::out|ofstream::trunc|ofstream::binary);
    if ( !os.is_open() )
    {
      gsErrorMsg("cannot open file '%s' for writing\n",outfile.c_str());
      return 1;
    }
    string name = get_base(outfile);
    write_lts_to_dot(l,os,name,print_state);
  }


  return 0;
}

