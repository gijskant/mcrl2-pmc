#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include "mcrl2/pbes/bisimulation.h"

using namespace std;
using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::pbes_system;
using namespace mcrl2::pbes_system::pbes_expr;
namespace po = boost::program_options;

std::string print_type(int type)
{
  switch (type)
  {
    case 0: return "branching bisimulation";
    case 1: return "strong bisimulation";
    case 2: return "weak bisimulation";
    case 3: return "branching simulation equivalence";
  }
  return "unknown type";
}

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  std::string infile1;            // location of model LPS
  std::string infile2;            // location of spec LPS
  std::string outfile;            // location of result
  int type;                       // the type of bisimulation
  bool normalize = false;         // normalize the result

  specification M;
  specification S;

  try {
    //--- reach options ---------
    boost::program_options::options_description bisimulation_options(
      "Usage: bisimulation [OPTION]... INFILE1 INFILE2 OUTFILE\n"
      "\n"
      "Read the LPS's from INFILE1 and INFILE2, compute a PBES that expresses\n"
      "bisimulation and write it to OUTFILE\n"
      "\n"
      "Four types of bisimulation are supported:\n"
      "  0 : branching bisimulation\n"
      "  1 : strong bisimulation\n"
      "  2 : weak bisimulation\n"
      "  3 : branching simulation equivalence\n"
      "\n"
      "Options"
    );
    bisimulation_options.add_options()
      ("help,h", "display this help")
      ("bisimulation,b", po::value<int> (&type)->default_value(0), "type of bisimulation")
      ("normalize,n", po::value<bool> (&normalize)->default_value(false)  , "normalize the result")
      ;

    //--- hidden options ---------
    po::options_description hidden_options;
    hidden_options.add_options()
      ("input-file1", po::value<std::string>(&infile1), "input file 1")
      ("input-file2", po::value<std::string>(&infile2), "input file 2")
      ("output-file", po::value<std::string>(&outfile), "output file")
    ;

    //--- positional options ---------
    po::positional_options_description positional_options;
    positional_options.add("input-file1", 1);
    positional_options.add("input-file2", 1);
    positional_options.add("output-file", 1);

    //--- command line options ---------
    po::options_description cmdline_options;
    cmdline_options.add(bisimulation_options).add(hidden_options);

    po::variables_map var_map;
    po::store(po::command_line_parser(argc, argv).
        options(cmdline_options).positional(positional_options).run(), var_map);
    po::notify(var_map);

    if (var_map.count("help")) {
      std::cout << bisimulation_options << "\n";
      return 1;
    }

    std::cout << "bisimulation parameters:" << std::endl;
    std::cout << "  input  file 1:         " << infile1 << std::endl;
    std::cout << "  input  file 2:         " << infile2 << std::endl;
    std::cout << "  output file  :         " << outfile << std::endl;
    std::cout << "  bisimulation :         " << print_type(type) << std::endl;
    std::cout << "  normalize    :         " << normalize << std::endl;

    M.load(infile1);
    S.load(infile2);
    pbes<> result;
    switch (type)
    {
      case 0: result = branching_bisimulation(M, S); break;
      case 1: result = strong_bisimulation(M, S); break;
      case 2: result = weak_bisimulation(M, S); break;
      case 3: result = branching_simulation_equivalence(M, S); break;
    }
    if (normalize)
    {
      result.normalize();
    }
    result.save(outfile);
  }
  catch(std::runtime_error e)
  {
    std::cerr << "runtime error: " << e.what() << std::endl;
    std::exit(1);
  }
  catch(std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 1;
  }
  catch(...) {
    std::cerr << "exception of unknown type!\n";
  }

  return 0;
}
