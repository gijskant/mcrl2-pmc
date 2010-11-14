// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file liblts.cpp

#include <string>
#include <set>
#include <stack>
#include <bitset>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdlib>
#include <algorithm>
// #include <boost/bind.hpp>
#include "aterm2.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/core/aterm_ext.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/core/parse.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lts/lts_utilities.h"
#include "mcrl2/lts/lts_algorithm.h"
#include "mcrl2/lts/lts_io.h"

using namespace mcrl2::core;
using namespace mcrl2::core::detail;

#ifdef USE_BCG
#include <bcg_user.h>
#endif

#define ATisAppl(x) (ATgetType(x) == AT_APPL)
#define ATisList(x) (ATgetType(x) == AT_LIST)

using namespace std;

namespace mcrl2
{
namespace lts
{
namespace detail
{

atermpp::vector < ATermAppl > state_label_lts::vector_templates;

lps::specification const& empty_specification() 
{
  static lps::specification dummy;

  return dummy;
}

lts_type guess_format(string const& s) 
{
  string::size_type pos = s.find_last_of('.');

  if ( pos != string::npos )
  {
    string ext = s.substr(pos+1);

    if ( ext == "aut" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected Aldebaran extension.\n";
      }
      return lts_aut;
    } 
    else if ( ext == "lts" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected mCRL2 extension.\n";
      }
      return lts_lts;
    } 
    else if ( ext == "svc" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected SVC extension; assuming mCRL2 format.\n";
      }
      return lts_lts;
    } 
    else if ( ext == "fsm" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected Finite State Machine extension.\n";
      }
      return lts_fsm;
    } 
    else if ( ext == "dot" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected GraphViz extension.\n";
      }
      return lts_dot;
#ifdef USE_BCG
    } 
    else if ( ext == "bcg" )
    {
      if (core::gsVerbose)
      { std::cerr << "Detected Binary Coded Graph extension.\n";
      }
      return lts_bcg;
#endif
    }
  }

  return lts_none;
}

static std::string type_strings[] = { "unknown", "lts", "aut", "fsm", "bcg", "dot", "svc" };

static std::string extension_strings[] = { "", "lts", "aut", "fsm", "bcg", "dot", "svc" };

static std::string type_desc_strings[] = { "unknown LTS format",
                                           "mCRL2 LTS format",
                                           "Aldebaran format (CADP)",
                                           "Finite State Machine format",
                                           "Binary Coded Graph format (CADP)" 
                                           "GraphViz format",
                                           "SVC format",
                                         };


static std::string mime_type_strings[] = { "", 
                                           "application/lts", 
                                           "text/aut", 
                                           "text/fsm", 
                                           "application/bcg", 
                                           "text/dot", 
                                           "application/svc"
                                         };

lts_type parse_format(std::string const& s) 
{
  if ( s == "lts")
  {
    return lts_lts;
  } 
  else if ( s == "aut" )
  {
    return lts_aut;
  } 
  else if ( s == "fsm" )
  {
    return lts_fsm;
  } 
#ifdef USE_BCG
  else if ( s == "bcg" )
  {
    return lts_bcg;
  }
#endif
  else if ( s == "dot" )
  {
    return lts_dot;
  } 
  else if ( s == "svc" )
  {
    return lts_svc;
  }  

  return lts_none;
}

std::string string_for_type(const lts_type type) 
{
  return (type_strings[type]);
}

std::string extension_for_type(const lts_type type) 
{
  return (extension_strings[type]);
}

std::string mime_type_for_type(const lts_type type) {
  return (mime_type_strings[type]);
}

static const std::set<lts_type> &initialise_supported_lts_formats()
{
  static std::set<lts_type> s;
  for (unsigned int i = lts_type_min; i<1+(unsigned int)lts_type_max; ++i)
  {
    if ( lts_none != (lts_type) i )
    {
      s.insert((lts_type) i);
    }
  }
  return s;
}
const std::set<lts_type> &supported_lts_formats()
{
  static const std::set<lts_type> &s = initialise_supported_lts_formats();
  return s;
}

std::string supported_lts_formats_text(lts_type default_format, const std::set<lts_type> &supported)
{
  vector<lts_type> types(supported.begin(),supported.end());
  std::sort(types.begin(),types.end(),boost::bind(lts_named_cmp<lts_type>,type_strings,_1,_2));

  string r;
  for (vector<lts_type>::iterator i=types.begin(); i!=types.end(); ++i)
  {
    r += "  '" + type_strings[*i] + "' for the " + type_desc_strings[*i];

    if ( *i == default_format )
    {
      r += " (default)";
    }

	// Still unsafe if types.size() < 2
	assert(types.size() >= 2);
    if ( i == types.end() - 2 )
    {
      r += ", or\n";
    } else if ( i != types.end() - 1)
    {
      r += ",\n";
    }
  }

  return r;
}

std::string supported_lts_formats_text(const std::set<lts_type> &supported)
{
  return supported_lts_formats_text(lts_none,supported);
}

std::string lts_extensions_as_string(const std::string &sep, const std::set<lts_type> &supported)
{
  vector<lts_type> types(supported.begin(),supported.end());
  std::sort(types.begin(),types.end(),boost::bind(lts_named_cmp<lts_type>,extension_strings,_1,_2));

  string r, prev;
  bool first = true;
  for (vector<lts_type>::iterator i=types.begin(); i!=types.end(); i++)
  {
    if ( extension_strings[*i] == prev ) // avoid mentioning extensions more than once
    {
      continue;
    }
    if ( first )
    {
      first = false;
    } else {
      r += sep;
    }
    r += "*." + extension_strings[*i];
    prev = extension_strings[*i];
  }

  return r;
}

std::string lts_extensions_as_string(const std::set<lts_type> &supported)
{
  return lts_extensions_as_string(",",supported);
}

} // namespace detail
} //lts
} //data
