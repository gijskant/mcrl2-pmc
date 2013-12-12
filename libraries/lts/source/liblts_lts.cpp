// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file liblts_lts.cpp

#include <string>
#include <sstream>
#include "svc/svc.h"
#include "mcrl2/core/nil.h"
#include "mcrl2/data/detail/io.h"
#include "mcrl2/lts/lts_lts.h"
#include "mcrl2/atermpp/aterm_int.h"

using namespace std;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::lts;
using namespace mcrl2::core::detail;
using namespace mcrl2::data::detail;
using namespace mcrl2::log;

static void read_from_lts(lts_lts_t& l, string const& filename)
{
  SVCfile f;
  SVCbool b;
  bool svc_file_has_state_info = false;

  if (SVCopen(&f,const_cast< char* >(filename.c_str()),SVCread,&b))
  {
    throw mcrl2::runtime_error("cannot open lts file '" + filename + "' for reading.");
  }

  string svc_type = SVCgetType(&f);
  if (svc_type == "mCRL2")
  {
    svc_file_has_state_info = false; // redundant.
  }
  else if (svc_type == "mCRL2+info")
  {
    svc_file_has_state_info = true;
  }
  else
  {
    throw mcrl2::runtime_error("lts file '" + filename + "' is not in the mCRL2 format.");
  }

  assert(SVCgetInitialState(&f)==0);
  if (svc_file_has_state_info)
  {
    using namespace mcrl2::data;
    using namespace mcrl2::lts::detail;
    aterm_appl state_label=(aterm_appl)SVCstate2ATerm(&f,(SVCstateIndex) SVCgetInitialState(&f));
    l.add_state(state_label_lts(state_label));
  }
  else
  {
    l.add_state();
  }
  l.set_initial_state((size_t) SVCgetInitialState(&f));


  SVCstateIndex from, to;
  SVClabelIndex label;
  SVCparameterIndex param;
  while (SVCgetNextTransition(&f,&from,&label,&to,&param))
  {
    size_t max = (from > to)?from:to;
    for (size_t i=l.num_states(); i<=max; i++)
    {
      if (svc_file_has_state_info)
      {
        using namespace mcrl2::data;
        using namespace mcrl2::lts::detail;
        aterm_appl state_label=(aterm_appl)SVCstate2ATerm(&f,(SVCstateIndex) i);
        l.add_state(state_label_lts(state_label));
      }
      else
      {
        l.add_state();
      }
    }

    for (size_t i=l.num_action_labels(); i<=(size_t)label; i++)
    {
      aterm_appl lab = (aterm_appl) SVClabel2ATerm(&f,(SVClabelIndex) i);
      l.add_action(lab,(aterm_cast<aterm_list>(lab[0]).empty())?true:false);
    }

    l.add_transition(transition((size_t) from,
                                (size_t) label,
                                (size_t) to));
  }

  SVCclose(&f);

  // Check to see if there is extra data at the end

  const std::string error_message="The .lts file " + filename +
                                  " does not appear to contain datatypes, action declarations and process parameters";
  ifstream g;
  g.open(filename.c_str(), std::ios::binary);
  g.seekg(-(12+8),ios_base::end);
  if (g.fail())
  {
    throw mcrl2::runtime_error(error_message + " (cannot reopen file)");
  }
  else
  {
    unsigned char buf[8+12];
    g.read((char*)buf,8+12);
    if (g.fail())
    {
      throw mcrl2::runtime_error(error_message + " (file does not contain control information)");
    }
    else
    {
      if (!strncmp(((char*) buf)+8,"   1STL2LRCm",12))
      {
        long position = 0;
        for (unsigned char i=0; i<8; i++)
        {
          position = (position << 8) + buf[7-i];
        }
        g.seekg(position, std::ios_base::beg);
        if (g.fail())
        {
          throw mcrl2::runtime_error(error_message + " (control information is incorrect)");
        }

        try
	{
	  aterm data=read_term_from_binary_stream(g);
          data = mcrl2::data::detail::add_index(data);
          data::data_specification data_spec(atermpp::aterm_appl(((aterm_appl)data)[0]));
          data_spec.declare_data_specification_to_be_type_checked(); // We can assume that this data spec is well typed.
          l.set_data(data::data_specification(data_spec));
          if (!core::is_nil((aterm_appl)((aterm_appl)data)[1]))
          {
            // The parameters below have the structure "ParamSpec(variable list);
            l.set_process_parameters(data::variable_list((aterm_cast<aterm_appl>(aterm_cast<aterm_appl>(data)[1]))[0]));
          }
          if (!core::is_nil((aterm_appl)((aterm_appl)data)[2]))
          {
            // The parameters below have the structure "ActSpec(variable list);
            l.set_action_labels(lps::action_label_list((aterm_cast<aterm_appl>(aterm_cast<aterm_appl>(data)[2]))[0]));
          }
        }
		catch(std::runtime_error& e)
		{
		  throw mcrl2::runtime_error(e.what() + std::string(" (data information is incorrect)"));
		}
      }
    }
    g.close();
  }
}

/* \brief Add an mCRL2 data specification, parameter list and action
 *         specification to a mCRL2 LTS in SVC format.
 * \param[in] filename   The file name of the mCRL2 LTS.
 * \param[in] data_spec  The data specification to add in mCRL2 internal
 *                       format (or NULL for none).
 * \param[in] params     The list of state(/process) parameters in mCRL2
 *                       internal format (or NULL for none).
 * \param[in] act_spec   The action specification to add in mCRL2 internal
 *                       format (or NULL for none).
 * \pre                  The LTS in filename is a mCRL2 SVC without extra
 *                       information. */
static void add_extra_mcrl2_lts_data(
  const std::string& filename,
  const bool has_data_spec,
  const aterm_appl& data_spec,
  const bool has_params,
  const aterm_list& params,
  const bool has_act_labels,
  const aterm_list& act_labels)
{
  std::fstream f(filename.c_str(), std::ios::in|std::ios::out|std::ios::binary|std::ios::app); // Open to append.
  if (f.fail())
  {
    throw mcrl2::runtime_error("Could not open file '" + filename + "' to add extra LTS information.");
    return;
  }

  aterm arg1 = (aterm)(has_data_spec?data_spec:atermpp::aterm_appl(core::detail::function_symbol_Nil()));
  aterm arg2 = (aterm)(has_params?aterm_appl(function_symbol("ParamSpec",1),(aterm) params):atermpp::aterm_appl(core::detail::function_symbol_Nil()));
  aterm arg3 = (aterm)(has_act_labels?atermpp::aterm_appl(core::detail::function_symbol_ActSpec(), act_labels):atermpp::aterm_appl(core::detail::function_symbol_Nil()));
  aterm data = (aterm) aterm_appl(function_symbol("mCRL2LTS1",3),arg1,arg2,arg3);
  data = mcrl2::data::detail::remove_index(data);

  /* Determine the position at which the additional information starts.
     Due to the way in which file operations are implemented on Windows, we need to
	 use the get pointer for determining the length of the SVC input. (seekp gives invalid
	 results, leading to a wrong encoded position in the output of the LTS file).
	 According to the example at http://www.cplusplus.com/reference/istream/istream/seekg/
	 this is the, more-or-less standard, way to determine the lenght of the file. */

  f.seekg (0, std::ios::end);
  long position = f.tellg();
  f.seekg (0, std::ios::beg);

  if (position == -1)
  {
    f.close();
    throw mcrl2::runtime_error("Could not determine file size of '" + filename +
                               "'; not adding extra information.");
    return;
  }

  write_term_to_binary_stream(data,f);
  if (f.fail())
  {
    f.close();
    throw mcrl2::runtime_error("Error writing extra LTS information to '" + filename +
                               "', file could be corrupted.");
    return;
  }

  unsigned char buf[8+12+1] = "XXXXXXXX   1STL2LRCm";
  for (size_t i=0; i<8; i++)
  {
    buf[i] = position % 0x100;
    position >>= 8;
  }

  f.write((char *)buf,8+12);
  if (f.fail())
  {
    f.close();
    throw mcrl2::runtime_error("error writing extra LTS information to '" + filename +
                               "', file could be corrupted.");
    return;
  }

  f.close();
}


static void write_to_lts(const lts_lts_t& l, string const& filename)
{
  SVCfile f;
  SVCbool b = l.has_state_info() ? SVCfalse : SVCtrue;
  if (SVCopen(&f,const_cast< char* >(filename.c_str()),SVCwrite,&b))
  {
    throw mcrl2::runtime_error("cannot open .lts file '" + filename + "' for writing.");
  }

  if (l.has_state_info())
  {
    SVCsetType(&f,const_cast < char* >("mCRL2+info"));
  }
  else
  {
    SVCsetType(&f,const_cast < char* >("mCRL2"));
  }

  SVCsetCreator(&f,const_cast < char* >("liblts (mCRL2)"));

  assert(l.initial_state()< ((size_t)1 << (sizeof(int)*8-1)));
  SVCsetInitialState(&f,SVCnewState(&f, l.has_state_info() ? (aterm)(aterm_appl)l.state_label(l.initial_state()) : (aterm)aterm_int(l.initial_state()) ,&b));

  SVCparameterIndex param = SVCnewParameter(&f,(aterm)aterm_list(),&b);

  const std::vector < transition> &trans=l.get_transitions();
  for (std::vector < transition>::const_iterator t=trans.begin(); t!=trans.end(); ++t)
  {
    assert(t->from()< ((size_t)1 << (sizeof(int)*8-1)));
    SVCstateIndex from = SVCnewState(&f, l.has_state_info() ? (aterm)(aterm_appl)l.state_label(t->from()) : (aterm) aterm_int(t->from()) ,&b);
    SVClabelIndex label = SVCnewLabel(&f, (aterm)l.action_label(t->label()).aterm_without_time(), &b);
    assert(t->to()< ((size_t)1 << (sizeof(int)*8-1)));
    SVCstateIndex to = SVCnewState(&f, l.has_state_info() ? (aterm)(aterm_appl)l.state_label(t->to()) : (aterm) aterm_int(t->to()) ,&b);
    SVCputTransition(&f,from,label,to,param);
  }

  SVCclose(&f);


  aterm_appl  data_spec = mcrl2::data::detail::data_specification_to_aterm_data_spec(l.data());
  aterm_list params = l.process_parameters();
  aterm_list act_spec = l.action_labels();
  add_extra_mcrl2_lts_data(filename,l.has_data(),data_spec,l.has_process_parameters(),params,l.has_action_labels(),act_spec);
}

namespace mcrl2
{
namespace lts
{

void lts_lts_t::save(const std::string& filename) const
{
  if (filename=="")
  {
    throw mcrl2::runtime_error("Cannot write svc/lts file " + filename + " to stdout");
  }
  else
  {
    mCRL2log(verbose) << "Starting to save file " << filename << "\n";
    write_to_lts(*this,filename);
  }
}

void lts_lts_t::load(const std::string& filename)
{
  if (filename=="")
  {
    throw mcrl2::runtime_error("Cannot read svc/lts file " + filename + " from stdin");
  }
  else
  {
    mCRL2log(verbose) << "Starting to load file " << filename << "\n";
    read_from_lts(*this,filename);
  }


}

}
}
