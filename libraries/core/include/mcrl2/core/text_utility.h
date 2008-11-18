// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/core/text_utility.h
/// \brief String manipulation functions.

#ifndef MCRL2_CORE_TEXT_UTILITY_H
#define MCRL2_CORE_TEXT_UTILITY_H

#include <fstream>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>

namespace mcrl2 {

namespace core {

  /// \brief Split a string into paragraphs.
  std::vector<std::string> split_paragraphs(const std::string& text)
  {
    std::vector<std::string> result;

    // find multiple line endings
    boost::xpressive::sregex paragraph_split = boost::xpressive::sregex::compile( "\\n\\s*\\n" );
  
    // the -1 below directs the token iterator to display the parts of
    // the string that did NOT match the regular expression.
    boost::xpressive::sregex_token_iterator cur( text.begin(), text.end(), paragraph_split, -1 );
    boost::xpressive::sregex_token_iterator end;

    for( ; cur != end; ++cur )
    {
      std::string paragraph = *cur;
      boost::trim(paragraph);
      if (paragraph.size() > 0)
      {
        result.push_back(paragraph);
      }
    }
    return result;
  }

  /// Split the text.
  inline
  std::vector<std::string> split(const std::string& line, const std::string& separators)
  {
    std::vector<std::string> result;
    boost::algorithm::split(result, line, boost::algorithm::is_any_of(separators));
    return result;
  }

  /// \brief Read text from a file.
  inline
  std::string read_text(const std::string& filename, bool warn=false)
  {
    std::ifstream in(filename.c_str());
    if (!in)
    {
      if (warn)
        std::cerr << "Could not open input file: " << filename << std::endl;
      return "";
    }
    in.unsetf(std::ios::skipws); //  Turn of white space skipping on the stream
  
    std::string s;
    std::copy(
      std::istream_iterator<char>(in),
      std::istream_iterator<char>(),
      std::back_inserter(s)
    );
  
    return s;
  }

  /// \brief Remove comments from a text (everything from '%' until end of line).
  inline
  std::string remove_comments(const std::string& text)
  {
    // matches everything from '%' until end of line
    boost::xpressive::sregex src = boost::xpressive::sregex::compile( "%[^\\n]*\\n" );

    std::string dest( "\n" );
    return boost::xpressive::regex_replace(text, src, dest);
  }

  /// \brief Removes whitespace from a string.
  inline
  std::string remove_whitespace(const std::string& text)
  {
    boost::xpressive::sregex src = boost::xpressive::sregex::compile("\\s");
    std::string dest("");
    return boost::xpressive::regex_replace(text, src, dest);
  }

  /// \brief Regular expression replacement in a string.
  inline
  std::string regex_replace(const std::string& src, const std::string& dest, const std::string& text)
  {
    return boost::xpressive::regex_replace(text, boost::xpressive::sregex::compile(src), dest);
  }

  /// \brief Split a string using a regular expression separator.
  std::vector<std::string> regex_split(const std::string& text, const std::string& sep)
  {
    std::vector<std::string> result;
    // find multiple line endings
    boost::xpressive::sregex paragraph_split = boost::xpressive::sregex::compile(sep);
    // the -1 below directs the token iterator to display the parts of
    // the string that did NOT match the regular expression.
    boost::xpressive::sregex_token_iterator cur( text.begin(), text.end(), paragraph_split, -1 );
    boost::xpressive::sregex_token_iterator end;
    for( ; cur != end; ++cur )
    {
      std::string word = *cur;
      boost::trim(word);
      if (word.size() > 0)
      {
        result.push_back(word);
      }
    }
    return result;
  }

} // namespace core

} // namespace mcrl2

#endif // MCRL2_CORE_TEXT_UTILITY_H
