#include "string_template.h"
#include <boost/xpressive/xpressive.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string/replace.hpp>
#ifndef MCRL2_USE_BOOST_INTERNAL
  #include <boost/regex.hpp>
#endif

using namespace boost;
using namespace boost::xpressive;

void StringTemplate::replace(string key, string value)
{
  string k_var = "%" + key + "%";

  //hardcoding replace_all(subject, k_var, value);
  int len = subject.size();
  int k_var_len = k_var.size();
  ostringstream new_subject;

  for(int i=0; i!=len; i++)
  {
    //find percent sign that marks start of variable
    if(subject[i]=='%')
    {
      //if the following chars indeed correspond to the key,
      // replace it by the value in the output;
      // and skip over the following characters.
      if(subject.compare(i, k_var_len, k_var)==0)
      {
        new_subject << value;
        //move to ending %-sign (-1 because "continue" still does the i++).
        i += k_var_len - 1;
        continue;
      }
    }
    new_subject << subject[i];
  }

  subject = new_subject.str();
}
/**
 * always returns false. this way, multiple has, replace_by pairs can be
 * combined in && and || clauses using short circuiting
 */
bool StringTemplate::replace_by(string value)
{
	replace(current_key, value);
	return false;
}
bool StringTemplate::has(string key)
{
	if(subject.find("%"+key+"%") != string::npos)
	{
		current_key = key;
		return true;
	}
	else
	{
		return false;
	}
}

ostringstream& StringTemplate::operator[] (string key)
{
  if(!streams.count(key))
  {
    boost::shared_ptr<ostringstream> os(new ostringstream());
    streams.insert(make_pair(key, os));
  }
  return *(streams[key]);
}

void StringTemplate::finalise()
{
  //pair<string, shared_ptr<ostringstream> > v;
  //BOOST_FOREACH(v, streams)
  for(map<string, boost::shared_ptr<ostringstream> >::iterator i = streams.begin(); i != streams.end(); i++)
  {
    string key(i->first);
    string value(i->second->str());
    this->replace(key, value);
  }
  // removes dangling commas, right next to a bracket.
  //

#ifdef MCRL2_USE_BOOST_INTERNAL  
     sregex rex = ((s1=as_xpr("[")|"(") >> *_s >> "," >> *_s) | (*_s >> "," >> *_s >> (s2=as_xpr(")")|"]"|":"));
     subject = regex_replace(subject, rex, std::string("$1$2"));
#else
      std::ostringstream t(std::ios::out | std::ios::binary);
      std::ostream_iterator<char, char> oi(t);

      boost::regex e1("\\[\\s*\\,\\s*");
      subject = regex_replace(subject , e1, "[");

      boost::regex e2("\\(\\s*\\,\\s*");
      subject = regex_replace(subject , e2, "(");

      boost::regex e3( "\\s*\\,\\s*\\]" );
      subject = regex_replace(subject , e3, "]");

      boost::regex e4( "\\s*\\,\\s*\\)" );
      subject = regex_replace(subject , e4, ")");

      boost::regex e5( "\\s*\\,\\s*:" );
      subject = regex_replace(subject , e5, ":");
#endif
}

StringTemplateFile::StringTemplateFile(string filecontent)
{
	//ifstream f(filename.c_str());

  istringstream f(filecontent);
	string fmtline;
	while(getline(f, fmtline))
	{


		//matches <key> = <value> where <key> must be a word and <value> is everything after the '='.
		//supports comments starting with f.ex. #
		// Original code:
		// -- begin --
		//
#ifdef MCRL2_USE_BOOST_INTERNAL  
		sregex rex = bos >> *_s >> (s1= +_w) >> *_s >> '=' >> !_s >> (s2= *_) >> !(as_xpr('\r')) >> eos;
		smatch matches;

		if(regex_match(fmtline, matches, rex))
		{
			format_strings[matches[1]] = matches[2];
		} 
		// -- end --
#else

		boost::regex rex( "\\s*(\\w+)(\\s*)=(\\s*)(.*)"); 
		boost::cmatch matches;

		if(boost::regex_match(fmtline.c_str(), matches, rex))
		{
/*			cout << "s :" << matches[0] << endl; // Returns original string if a match is found
			cout << "k :" << matches[1] << endl; // Key 
			cout << "v :" << matches[4] << endl; // Value */
			format_strings[matches[1]] = matches[4];
		}
#endif

	}
}

string StringTemplateFile::fmt_string(string id)
{
	if(!format_strings.count(id))
	{
		throw "Cannot find key '" + id + "' in formatting file";
	}
	return format_strings.find(id)->second;
}
StringTemplate StringTemplateFile::get(string id)
{
	return StringTemplate(fmt_string(id));
}

string StringTemplateFile::all(string id, map<string, string> substitutions)
{
	StringTemplate sf(get(id));

	pair<string, string> subst;
	BOOST_FOREACH(subst, substitutions)
	{
		sf.replace(subst.first, subst.second);
	}
	return sf.get();
}

