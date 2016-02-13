// Author(s): Muck van Weerdenburg, Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file liblts_aut.cpp

#include <string>
#include <sstream>
#include <fstream>
#include "mcrl2/lts/lts_aut.h"
#include "mcrl2/lts/detail/liblts_swap_to_from_probabilistic_lts.h"


using namespace mcrl2::lts;
using namespace std;


static void read_newline(istream& is, const size_t lineno)
{
  char ch;
  is.get(ch);

  // Skip over spaces
  while (ch == ' ')
  {
    is.get(ch);
  }

  // Windows systems typically have a carriage return before a newline.
  if (ch == '\r')
  {
    is.get(ch);
  }

  if (ch != '\n')
  {
    if (lineno==1)
    {
      throw mcrl2::runtime_error("Expect a newline after the header des(...,...,...).");
    }
    else
    {
      throw mcrl2::runtime_error("Expect a newline after the transition at line " + std::to_string(lineno) + ".");
    }
  }
}

// reads a number, puts it in s, and reads one extra character, which must be either a space or a closing bracket.
static void read_natural_number_to_string(istream& is, string& s, const size_t lineno)
{
  assert(s.empty());
  char ch;
  is >> skipws >> ch;
  for( ; isdigit(ch) ; is.get(ch))
  {
    s.push_back(ch);
  }
  is.putback(ch);
  if (s.empty())
  {
    throw mcrl2::runtime_error("Expect a number at line " + std::to_string(lineno) + ".");
  }
}

static size_t find_label_index(const string& s, map < string, size_t >& labs, probabilistic_lts_aut_t& l)
{
  size_t label;

  const map < string, size_t >::const_iterator i=labs.find(s);
  if (i==labs.end())
  {
    label=l.add_action(s,s=="tau");
    labs[s]=label;
  }
  else
  {
    label=i->second;
  }
  return label;
}

void add_state(size_t& state,
               map <size_t,size_t>& state_number_translator)
{
  map <size_t,size_t>::const_iterator j=state_number_translator.find(state);
  if (j==state_number_translator.end())
  {
    // Not found.
    const size_t new_state_number=state_number_translator.size(); 
    state_number_translator[state]=new_state_number;
    state=new_state_number;
  }
  else
  {
    // found.
    state=j->second;
  }
} 

void add_states(detail::lts_aut_base::probabilistic_state& probability_state,
                map <size_t,size_t>& state_number_translator)
{
  vector < detail::lts_aut_base::state_probability_pair> result;
  for(const detail::lts_aut_base::state_probability_pair& p: probability_state)
  {
    size_t new_state_index=p.state();
    add_state(new_state_index,state_number_translator);
    result.push_back(detail::lts_aut_base::state_probability_pair(new_state_index,p.probability())); 
  }
  probability_state=detail::lts_aut_base::probabilistic_state(result.begin(), result.end());
}
   

// This procedure tries to read states, indicated by numbers
// with in between fractions of the shape number/number. The
// last state number is put in state. The remainder as pairs
// in the vector. Typical expected input is 3 2/3 4 1/6 78 1/6 3.
static mcrl2::lts::probabilistic_lts_aut_t::probabilistic_state_t read_probabilistic_state(
  istream& is,
  const size_t lineno)
{
  vector<detail::lts_aut_base::state_probability_pair> additional_probabilistic_states;
  mcrl2::lts::probabilistic_arbitrary_precision_fraction remainder=mcrl2::lts::probabilistic_arbitrary_precision_fraction::one();
  size_t state;
  bool ready=false;
  while (is.good() && !ready)
  {
    is >> skipws >> state;

    if (!is.good())
    {
      throw mcrl2::runtime_error("Expect a state number at line " + std::to_string(lineno) + ".");
    }

    // Check whether the next character is a comma.
    char ch;
    is >> skipws >> ch;
    is.putback(ch);

    if (!isdigit(ch))
    {
      ready=true;
    }
    else
    {
      // Now attempt to read probabilities followed by a state.
      string enumerator;
      read_natural_number_to_string(is,enumerator,lineno);
      char ch;
      is >> skipws >> ch;
      if (ch != '/')
      {
        throw mcrl2::runtime_error("Expect a / in a probability at line " + std::to_string(lineno) + ".");
      }

      string denominator;
      read_natural_number_to_string(is,denominator,lineno);
      mcrl2::lts::probabilistic_arbitrary_precision_fraction frac(enumerator,denominator);
      remainder=remainder-frac;
      additional_probabilistic_states.push_back(detail::lts_aut_base::state_probability_pair(state, frac));
    }
  }
  if (additional_probabilistic_states.size()==0)
  {
    return detail::lts_aut_base::probabilistic_state(state);
  }
  additional_probabilistic_states.push_back(detail::lts_aut_base::state_probability_pair(state, remainder));
  return detail::lts_aut_base::probabilistic_state(additional_probabilistic_states.begin(),additional_probabilistic_states.end());
}


static void read_aut_header(
  istream& is,
  detail::lts_aut_base::probabilistic_state& initial_state,
  size_t& num_transitions,
  size_t& num_states)
{
  string s;
  is.width(3);
  is >> skipws >> s;

  if (s!="des")
  {
    throw mcrl2::runtime_error("Expect an .aut file to start with 'des'.");
  }

  char ch;
  is >> skipws >> ch;

  if (ch != '(')
  {
    throw mcrl2::runtime_error("Expect an opening bracket '(' after 'des' in the first line of a .aut file.");
  }

  // is >> skipws >> initial_state;
  initial_state=read_probabilistic_state(is,1);

  is >> skipws >> ch;
  if (ch != ',')
  {
    throw mcrl2::runtime_error("Expect a comma after the first number in the first line of a .aut file.");
  }

  is >> skipws >> num_transitions;

  is >> skipws >> ch;
  if (ch != ',')
  {
    throw mcrl2::runtime_error("Expect a comma after the second number in the first line of a .aut file.");
  }

  is >> skipws >> num_states;

  is >> ch;

  if (ch != ')')
  {
    throw mcrl2::runtime_error("Expect a closing bracket ')' after the third number in the first line of a .aut file.");
  }

  read_newline(is,1);
}

static bool read_aut_transition(
  istream& is,
  size_t& from,
  string& label,
  detail::lts_aut_base::probabilistic_state& target_probabilistic_state,
  const size_t lineno)
{
  char ch;
  is >> skipws >> ch;
  if (is.eof())
  {
    return false;
  }
  if (ch != '(')
  {
    throw mcrl2::runtime_error("Expect opening bracket at line " + std::to_string(lineno) + ".");
  }

  is >> skipws >> from;

  is >> skipws >> ch;
  if (ch != ',')
  {
    throw mcrl2::runtime_error("Expect that the first number is followed by a comma at line " + std::to_string(lineno) + ".");
  }

  is >> skipws >> ch;
  if (ch == '"')
  {
    label="";
    is >> ch;
    while ((ch != '"') && !is.eof())
    {
      label.push_back(ch);
      is >> ch;
    }

    if (ch != '"')
    {
      throw mcrl2::runtime_error("Expect that the second item is a quoted label (using \") at line " + std::to_string(lineno) + ".");
    }
    is >> skipws >> ch;
  }
  else
  {
    label = ch;
    is >> ch;
    while ((ch != ',') && !is.eof())
    {
      label.push_back(ch);
      is >> ch;
    }
  }

  if (ch != ',')
  {
    throw mcrl2::runtime_error("Expect a comma after the quoted label at line " + std::to_string(lineno) + ".");
  }

  // is >> skipws >> to;
  target_probabilistic_state=read_probabilistic_state(is,lineno);

  is >> ch;
  if (ch != ')')
  {
    throw mcrl2::runtime_error("Expect a closing bracket at the end of the transition at line " + std::to_string(lineno) + ".");
  }

  read_newline(is,lineno);
  return true;
}

static void read_from_aut(probabilistic_lts_aut_t& l, istream& is)
{
  size_t line_no = 1;
  size_t ntrans=0, nstate=0;

  detail::lts_aut_base::probabilistic_state initial_probabilistic_state;
  read_aut_header(is,initial_probabilistic_state,ntrans,nstate);

  map <size_t,size_t> state_number_translator;
  add_states(initial_probabilistic_state,state_number_translator);

  if (nstate==0)
  {
    throw mcrl2::runtime_error("cannot parse AUT input that has no states; at least an initial state is required.");
  }

  l.set_num_states(nstate,false);
  l.clear_transitions(ntrans); // Reserve enough space for the transitions.
  
  map < string, size_t > action_labels;
  size_t initial_state=l.add_probabilistic_state(initial_probabilistic_state);
  assert(initial_state==0);
  l.set_initial_probabilistic_state(initial_state);

  while (!is.eof())
  {
    size_t from;
    detail::lts_aut_base::probabilistic_state probabilistic_target_state;
    string s;

    line_no++;

    if (!read_aut_transition(is,from,s,probabilistic_target_state,line_no))
    {
      break; // eof encountered
    }

    add_state(from,state_number_translator); // This can change the number of from.
    add_states(probabilistic_target_state,state_number_translator);
    size_t probabilistic_state_index=l.add_probabilistic_state(probabilistic_target_state);

    l.add_transition(transition(from,find_label_index(s,action_labels,l),probabilistic_state_index));
    
    if (state_number_translator.size() > l.num_states())
    {
      throw mcrl2::runtime_error("Number of actual states in .aut file is higher than maximum (" +
                                 std::to_string(l.num_states()) + ") given by header (found at line " + std::to_string(line_no) + ").");
    }
  }

  if (ntrans != l.num_transitions())
  {
    throw mcrl2::runtime_error("number of transitions read (" + std::to_string(l.num_transitions()) +
                               ") does not correspond to the number of transition given in the header (" + std::to_string(ntrans) + ").");
  }
}


static void write_probabilistic_state(const detail::lts_aut_base::probabilistic_state prob_state, ostream& os)
{
  mcrl2::lts::probabilistic_arbitrary_precision_fraction previous_probability;
  bool first_element=true;
  for (const detail::lts_aut_base::state_probability_pair& p: prob_state)
  {
    if (first_element)
    {
      os << p.state();
      previous_probability=p.probability();
      first_element=false;
    }
    else
    {
      os << " " << pp(previous_probability) << " " << p.state();
      previous_probability=p.probability();
    }
  }
}

static void write_to_aut(const probabilistic_lts_aut_t& l, ostream& os)
{
  os << "des (";
  write_probabilistic_state(l.initial_probabilistic_state(),os);

  os << "," << l.num_transitions() << "," << l.num_states() << ")" << endl;

  for (const transition& t: l.get_transitions())
  {
    os << "(" << t.from() << ",\"" << pp(l.action_label(t.label())) << "\",";
    write_probabilistic_state(l.probabilistic_state(t.to()),os);
    os << ")" << endl;
  }
}

namespace mcrl2
{
namespace lts
{

void probabilistic_lts_aut_t::load(const string& filename)
{
  if (filename=="")
  {
    read_from_aut(*this,cin);
  }
  else
  {
    ifstream is(filename.c_str());

    if (!is.is_open())
    {
      throw mcrl2::runtime_error("cannot open .aut file '" + filename + ".");
    }

    read_from_aut(*this,is);
    is.close();
  }
}

void probabilistic_lts_aut_t::load(istream& is)
{
  read_from_aut(*this,is);
}

void probabilistic_lts_aut_t::save(string const& filename) const
{
  if (filename=="")
  {
    write_to_aut(*this,cout);
  }
  else
  {
    ofstream os(filename.c_str());

    if (!os.is_open())
    {
      throw mcrl2::runtime_error("cannot create .aut file '" + filename + ".");
      return;
    }
    write_to_aut(*this,os);
    os.close();
  }
}

void lts_aut_t::load(const string& filename)
{
  probabilistic_lts_aut_t l;
  l.load(filename);
  detail::swap_to_non_probabilistic_lts
             <state_label_empty, 
              action_label_string, 
              detail::lts_aut_base::probabilistic_state,
              detail::lts_aut_base>(l,*this);
}

void lts_aut_t::load(istream& is)
{
  probabilistic_lts_aut_t l;
  l.load(is);
  detail::swap_to_non_probabilistic_lts
             <state_label_empty, 
              action_label_string, 
              detail::lts_aut_base::probabilistic_state,
              detail::lts_aut_base>(l,*this);
}

void lts_aut_t::save(string const& filename) const
{
  probabilistic_lts_aut_t l;
  detail::translate_to_probabilistic_lts
            <state_label_empty, 
             action_label_string, 
             detail::lts_aut_base::probabilistic_state, 
             detail::lts_aut_base>(*this,l);
  l.save(filename);
}


}
}
