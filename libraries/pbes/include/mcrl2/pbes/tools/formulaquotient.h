// Author(s): Gijs Kant
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/tools/formulaquotient.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_TOOLS_FORMULAQUOTIENT_H
#define MCRL2_PBES_TOOLS_FORMULAQUOTIENT_H

#include <algorithm>
#include <fstream>

#include <boost/timer.hpp>

#include "mcrl2/lps/io.h"
#include "mcrl2/lps/network.h"
#include "mcrl2/lps/parse.h"
#include "mcrl2/lps/quotienting.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/synchronization_vector.h"
#include "mcrl2/modal_formula/algorithms.h"
#include "mcrl2/modal_formula/formula_size.h"
#include "mcrl2/modal_formula/modal_equation_system.h"
#include "mcrl2/modal_formula/parelm.h"
#include "mcrl2/modal_formula/quotienting.h"
#include "mcrl2/modal_formula/rewrite_quantifiers.h"
#include "mcrl2/modal_formula/simplifying_rewriter.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/transform_modal_equation_system.h"
#include "mcrl2/modal_formula/unfold_recursion.h"
#include "mcrl2/pbes/approximation.h"
#include "mcrl2/pbes/io.h"
#include "mcrl2/pbes/lps2pbes.h"
#include "mcrl2/pbes/mes2pbes.h"
#include "mcrl2/utilities/logger.h"

namespace mcrl2 {

namespace state_formulas {

static const std::string MES_EXT = ".mes";

struct formulaquotient_options
{
  formulaquotient_options() :
    iterative(false),
    parelm(false),
    simplify(false),
    unfold_recursion(false),
    write_intermediate_formulas(false),
    write_intermediate_networks(false)
  {  }

  bool iterative;
  bool parelm;
  bool simplify;
  bool unfold_recursion;
  bool write_intermediate_formulas;
  bool write_intermediate_networks;
};


inline void enrich_lps(lps::specification& lps, const std::set<process::action_label>& action_labels)
{
  std::set<process::action_label> lps_labels(lps.action_labels().begin(), lps.action_labels().end());
  std::set<process::action_label> vector_labels;
  std::set_difference(action_labels.begin(), action_labels.end(), lps_labels.begin(), lps_labels.end(),
                        std::inserter(vector_labels, vector_labels.begin()));
  // add action labels
  for(auto it = vector_labels.begin(); it != vector_labels.end(); ++it)
  {
    //mCRL2log(log::debug) << "adding action label: " << pp(*it) << ": " << pp((*it).sorts()) << "." << std::endl;
    lps.action_labels().push_front(*it);
  }
}

void enrich_data_specification(data::data_specification& data, const data::data_specification& other)
{
  for(auto s: other.user_defined_sorts())
  {
    data.add_sort(s);
  }
  for(auto c: other.user_defined_constructors())
  {
    data.add_constructor(c);
  }
  for(auto m: other.user_defined_mappings())
  {
    data.add_mapping(m);
  }
  for(auto e: other.user_defined_equations())
  {
    data.add_equation(e);
  }
  for(auto a: other.user_defined_aliases())
  {
    data.add_alias(a);
  }
}

inline void read_formula_from_file(const std::string& input_filename, state_formulas::state_formula& formula, lps::specification& lps)
{
  // load formula file
  boost::timer t;
  if (input_filename.empty())
  {
    mCRL2log(log::verbose) << "reading formula from stdin..." << std::endl;
    formula = state_formulas::algorithms::parse_state_formula(std::cin, lps);
  }
  else
  {
    mCRL2log(log::verbose) << "reading formula from file '" <<  input_filename << "'..." << std::endl;
    std::ifstream instream(input_filename.c_str(), std::ifstream::in);
    if (!instream)
    {
      throw mcrl2::runtime_error("cannot open input file: " + input_filename);
    }

    formula = state_formulas::algorithms::parse_state_formula(instream, lps);
    instream.close();
  }
  mCRL2log(log::verbose) << "(reading formula took " <<  t.elapsed() << " seconds)" << std::endl;
}

inline void write_formula_to_file(const std::string& output_filename, const state_formulas::state_formula& formula)
{
  // save the quotient formula
  if (output_filename.empty())
  {
    mCRL2log(log::verbose) << "writing quotient formula to stdout..." << std::endl;
  }
  else
  {
    mCRL2log(log::verbose) << "writing quotient formula to file '" <<  output_filename << "'..." << std::endl;
  }
  if (output_filename.empty())
  {
    std::cout << pp(formula) << std::endl;
  }
  else
  {
    std::ofstream outstream(output_filename.c_str(), std::ofstream::out);
    if (!outstream)
    {
      throw mcrl2::runtime_error("cannot open output file: " + output_filename);
    }
    outstream << pp(formula) << std::endl;
    outstream.close();
  }
}

inline void write_mes_to_file(const std::string& output_filename, const state_formulas::modal_equation_system& mes)
{
  // save the modal equation system
  if (output_filename.empty())
  {
    mCRL2log(log::verbose) << "writing modal equation system to stdout..." << std::endl;
  }
  else
  {
    mCRL2log(log::verbose) << "writing modal equation system to file '" <<  output_filename << "'..." << std::endl;
  }
  if (output_filename.empty())
  {
    std::cout << pp(mes) << std::endl;
  }
  else
  {
    std::ofstream outstream(output_filename.c_str(), std::ofstream::out);
    if (!outstream)
    {
      throw mcrl2::runtime_error("cannot open output file: " + output_filename);
    }
    outstream << pp(mes) << std::endl;
    outstream.close();
  }
}

void formulaquotient(const std::string& input_filename,
              const std::string& output_filename,
              const std::string& input_network_filename,
              const std::string& output_network_filename,
              const std::string& lps_filename,
              const std::string& synchronization_vector_filename,
              size_t i,
              formulaquotient_options tool_options,
              quotienting_options algorithm_options
             )
{
  if (tool_options.iterative)
  {
    // determine filename parts for storing intermediate results
    std::string input_name;
    std::string input_ext;
    size_t lastslash = input_filename.find_last_of("/");
    if (lastslash == std::string::npos)
    {
      input_name = input_filename;
    }
    else
    {
      input_name = input_filename.substr(lastslash+1, input_filename.size());
    }
    size_t lastdot = input_name.find_last_of(".");
    if (lastdot == std::string::npos)
    {
      input_ext = "";
    }
    else
    {
      input_ext = input_name.substr(lastdot, input_name.size());
      input_name = input_name.substr(0, lastdot);
    }
    std::string network_name;
    std::string network_ext;
    lastdot = input_network_filename.find_last_of(".");
    if (lastdot == std::string::npos)
    {
      network_ext = "";
      network_name = input_network_filename;
    }
    else
    {
      network_ext = input_network_filename.substr(lastdot, input_network_filename.size());
      network_name = input_network_filename.substr(0, lastdot);
    }

    data::data_specification data_spec;
    lps::network network;
    state_formulas::modal_equation_system mes;
    if (input_network_filename.empty())
    {
      throw std::runtime_error("No network file specified. Cannot apply iterative quotienting.");
    }
    // read network of LPSs from file
    mCRL2log(log::verbose) << "reading network from file '" <<  input_network_filename << "'..." << std::endl;
    network.load(input_network_filename);
    size_t n = network.lps_filenames().size();
    bool early_result = false;
    for (size_t j = 0; j < n && !early_result; j++)
    {
      boost::timer timer;
      lps::specification lps;
      lps::synchronization_vector v;
      mCRL2log(log::verbose) << std::endl << "Quotienting component " << j << ":" << std::endl;
      //mCRL2log(log::verbose) << "network: " << pp(network) << std::endl;
      mCRL2log(log::verbose) << "reading LPS at index " << 0 << " from file '"
          <<  network.lps_filenames()[0] << "'..." << std::endl;
      load_lps(lps, network.lps_filenames()[0]);
      if (j==0)
      {
        network.synchronization_vector().normalize(lps.data());
        enrich_lps(lps, network.synchronization_vector().action_labels());
        log::log_level_t level = log::mcrl2_logger::get_reporting_level();
        log::mcrl2_logger::set_reporting_level(log::info);
        state_formulas::state_formula formula;
        read_formula_from_file(input_filename, formula, lps);
        mCRL2log(log::verbose) << "Transforming formula..." << std::endl; // << pp(formula) << std::endl;
        mes = state_formulas::transform(formula);
        std::stringstream filename_s;
        filename_s << input_name << MES_EXT; // input_ext;
        write_mes_to_file(filename_s.str(), mes);
        //mCRL2log(log::verbose) << "Equation system:" << std::endl << pp(mes) << std::endl;
        log::mcrl2_logger::set_reporting_level(level);
      }
      v = network.synchronization_vector();
      //state_formula result_formula;
      label_generator label_generator;
      {
        boost::timer t;
        mCRL2log(log::verbose) << "computing quotient formula for the process at index " << 0 << "..." << std::endl;
        state_formulas::quotient_builder f(lps, v, 0, algorithm_options);
        f.add_identifiers(mes);
        //result_formula = f(formula);
        f.update(mes);
        //mCRL2log(log::verbose) << "Quotienting result:" << std::endl << pp(mes) << std::endl;
        mCRL2log(log::verbose) << "(quotienting took " <<  t.elapsed() << " seconds)" << std::endl;
        mCRL2log(log::verbose) << "(" << f.report_cache_hits() << ")" << std::endl;
        label_generator = f.label_generator();
      }
      size_t size1 = state_formulas::formula_size(mes);

      if (tool_options.parelm)
      {
        boost::timer t;
        // applying parameter elimination
        mCRL2log(log::verbose) << "removing unused parameters..." << std::endl;
        log::log_level_t level = log::mcrl2_logger::get_reporting_level();
        log::mcrl2_logger::set_reporting_level(log::warning);
        state_formulas::state_formula_parelm_algorithm algorithm;
        //algorithm.run(result_formula);
        algorithm.run(mes);
        log::mcrl2_logger::set_reporting_level(level);
        mCRL2log(log::verbose) << "(parelm took " <<  t.elapsed() << " seconds)" << std::endl;
      }

      if (tool_options.simplify)
      {
        boost::timer t;
        // simplifying formula
        mCRL2log(log::verbose) << "rewriting formula..." << std::endl;
        //result_formula = state_formulas::rewrite_quantifiers(result_formula, lps.data());
        state_formulas::rewrite_quantifiers(mes, lps.data());
        //{
        //        std::stringstream filename_s;
        //        filename_s << input_name << "_tmp_" << j << input_ext;
        //        write_formula_to_file(filename_s.str(), result_formula);
        // }
        // result_formula = state_formulas::simplify(result_formula, lps.data());
        state_formulas::simplify(mes, lps.data());
        mCRL2log(log::verbose) << "(simplify took " <<  t.elapsed() << " seconds)" << std::endl;
        size_t size2 = state_formulas::formula_size(mes);
        mCRL2log(log::verbose) << "(formula reduced to " << ((size1==0) ? 100 : (((float)size2/(float)size1)*100)) << " %)"<< std::endl;
      }

      if (tool_options.unfold_recursion)
      {
        boost::timer t;
        // unfold unguarded recursion
        mCRL2log(log::verbose) << "unfolding unguarded recursion..." << std::endl;
        state_formulas::unfold_recursion(mes, lps.data());
        mCRL2log(log::verbose) << "(unfolding took " <<  t.elapsed() << " seconds)" << std::endl;

        t.restart();
        // applying parameter elimination
        mCRL2log(log::verbose) << "removing unused parameters..." << std::endl;
        log::log_level_t level = log::mcrl2_logger::get_reporting_level();
        log::mcrl2_logger::set_reporting_level(log::warning);
        state_formulas::state_formula_parelm_algorithm algorithm;
        algorithm.run(mes);
        log::mcrl2_logger::set_reporting_level(level);
        mCRL2log(log::verbose) << "(parelm took " <<  t.elapsed() << " seconds)" << std::endl;

        t.restart();
        // simplifying formula
        mCRL2log(log::verbose) << "rewriting formula..." << std::endl;
        state_formulas::rewrite_quantifiers(mes, lps.data());
        state_formulas::simplify(mes, lps.data());
        // FIXME result_formula = state_formulas::discover_parameters(result_formula);
        state_formulas::simplify(mes, lps.data());
        mCRL2log(log::verbose) << "(simplify took " <<  t.elapsed() << " seconds)" << std::endl;
      }

      lps::network result_network;
      {
        boost::timer t;
        mCRL2log(log::verbose) << "computing quotient network..." << std::endl;
        result_network = lps::quotient(network, 0, label_generator);
        mCRL2log(log::verbose) << "(computing network took " <<  t.elapsed() << " seconds)" << std::endl;
      }

      //formula = result_formula;
      network = result_network;

      mCRL2log(log::verbose) << "Quotienting component " << j << " took " << timer.elapsed() << " seconds." << std::endl;

      if (j == 0)
      {
        // get data specification for generating the pbes
        data_spec = lps.data();
      }
      else
      {
        enrich_data_specification(data_spec, lps.data());
      }

      if (tool_options.write_intermediate_formulas)
      {
        std::stringstream filename_s;
        filename_s << input_name << "_" << j << MES_EXT; // input_ext;
        write_mes_to_file(filename_s.str(), mes);

        pbes_system::pbes intermediate_pbes = state_formulas::mes2pbes_approximate(data_spec, mes);
        filename_s.str(""); // clear stream
        filename_s << input_name << "_" << j << ".pbes";
        save_pbes(intermediate_pbes, filename_s.str());

        // (partially) solve the PBES using Gauss approximation
        bool result_under = pbes_partial_solve(intermediate_pbes, false);
        mCRL2log(log::verbose) << "Result for overapproximation: " << (result_under ? "true":"false") << std::endl;
        if (result_under)
        {
          early_result = true;
          // construct dummy equation system representing 'true'
          core::identifier_string t("T");
          variable init(t, data::data_expression_list());
          modal_equation eq(fixpoint_symbol::nu(), t, data::variable_list(), state_formulas::true_());
          std::vector<modal_equation> eq_list;
          eq_list.push_back(eq);
          mes = state_formulas::modal_equation_system(init, eq_list);
          mCRL2log(log::info) << "Formula is true." << std::endl;
        }
        // TODO: if not solved, transfer partial results to formula for reduction

        /*
        intermediate_pbes = state_formulas::mes2pbes_approximate(data_spec, mes, false);
        filename_s.str(""); // clear stream
        filename_s << input_name << "_" << j << "_F.pbes";
        save_pbes(intermediate_pbes, filename_s.str());

        bool result_over = pbes_partial_solve(intermediate_pbes, true);
        mCRL2log(log::verbose) << "Result for underapproximation: " << (result_over ? "true":"false") << std::endl;
        if (result_over)
        {
          early_result = true;
          // construct dummy equation system representing 'false'
          core::identifier_string f("F");
          variable init(f, data::data_expression_list());
          modal_equation eq(fixpoint_symbol::mu(), f, data::variable_list(), state_formulas::false_());
          std::vector<modal_equation> eq_list;
          eq_list.push_back(eq);
          mes = state_formulas::modal_equation_system(init, eq_list);
          mCRL2log(log::info) << "Formula is false." << std::endl;
        }
        */

      }
      if (tool_options.write_intermediate_networks)
      {
        std::stringstream filename_s;
        filename_s << network_name << "_" << j << network_ext;
        network.save(filename_s.str());
      }

    }

    // lps2pbes
    /*
    std::string delta_spec =
        "proc P = delta;"
        ""
        "init P;";
    lps::specification delta_lps = lps::parse_linear_process_specification(delta_spec);
    delta_lps.data() = data_spec;
    */
    mCRL2log(log::verbose) << "converting state formula and LPS to a PBES..." << std::endl;
    pbes_system::pbes result_pbes;
    {
      boost::timer t;
      // result_pbes = pbes_system::lps2pbes(delta_lps, formula);
      result_pbes = state_formulas::mes2pbes(data_spec, mes);
      mCRL2log(log::verbose) << "(converting to PBES took " <<  t.elapsed() << " seconds)" << std::endl;
    }
    save_pbes(result_pbes, output_filename);
    //write_formula_to_file(output_filename, formula);
    mCRL2log(log::verbose) << "done." << std::endl;
  }
  else
  {
    lps::specification lps;
    lps::synchronization_vector v;
    lps::network network;
    std::set<process::action_label> action_labels;
    if (!input_network_filename.empty())
    {
      // read network of LPSs from file
      mCRL2log(log::verbose) << "reading network from file '" <<  input_network_filename << "'..." << std::endl;
      network.load(input_network_filename);
      if (i < network.lps_filenames().size())
      {
        mCRL2log(log::verbose) << "reading LPS at index " << i << " from file '"
            <<  network.lps_filenames()[i] << "'..." << std::endl;
        load_lps(lps, network.lps_filenames()[i]);
      }
      else
      {
        throw std::runtime_error("Index should be in the range [0 .. n-1], where n is the size of the network.");
      }
      network.synchronization_vector().normalize(lps.data());
      v = network.synchronization_vector();
      action_labels = v.action_labels();
    }
    else
    {
      // read LPS and synchronization vector from file
      if (lps_filename.empty())
      {
        throw mcrl2::runtime_error("options -n and -p not specified (one of those is required).");
      }
      if (synchronization_vector_filename.empty())
      {
        throw mcrl2::runtime_error("option -s is not specified");
      }
      // load LPS
      mCRL2log(log::verbose) << "reading LPS from file '" <<  lps_filename << "'..." << std::endl;
      load_lps(lps, lps_filename);
      // load synchronization vector
      mCRL2log(log::verbose) << "reading synchronization vector from file '" <<  synchronization_vector_filename << "'..." << std::endl;
      v.load(synchronization_vector_filename);
      v.normalize(lps.data());
      mCRL2log(log::debug) << "synchronization vector: " << std::endl;
      mCRL2log(log::debug) << pp(v) << std::endl;
    }
    enrich_lps(lps, action_labels);
    // load formula file
    state_formulas::state_formula formula;
    read_formula_from_file(input_filename, formula, lps);

    boost::timer t;
    mCRL2log(log::verbose) << "computing quotient formula for the process at index " << i << "..." << std::endl;
    //state_formula result = state_formulas::algorithms::quotient(formula, lps, v, i);
    state_formulas::quotient_builder f(lps, v, i, algorithm_options);
    f.add_identifiers(formula);
    state_formula result_formula = f.apply(formula);
    mCRL2log(log::verbose) << "(computing quotient took " <<  t.elapsed() << " seconds)" << std::endl;

    // compute quotient network
    if (!output_network_filename.empty())
    {
      label_generator label_generator = f.label_generator();
      mCRL2log(log::verbose) << "computing quotient network..." << std::endl;
      lps::network result_network = lps::quotient(network, i, label_generator);
      mCRL2log(log::verbose) << "writing quotient network to file '" <<  output_network_filename << "'..." << std::endl;
      result_network.save(output_network_filename);
    }
    write_formula_to_file(output_filename, result_formula);
    mCRL2log(log::verbose) << "done." << std::endl;
  }
}

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_PBES_TOOLS_FORMULAQUOTIENT_H
