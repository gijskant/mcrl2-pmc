#include <sstream>
#include <iostream>
#include <cstdio>

#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_log.hpp>
#include <boost/test/unit_test_monitor.hpp>

#include <sip/detail/basic_messenger.tcc>
#include <sip/detail/command_line_interface.h>
#include <sip/tool.h>
#include <sip/controller.h>

#define BOOST_TEST_SHOW_PROGRESS yes

using boost::unit_test::test_suite;

using namespace transport;
using namespace sip;
using namespace sip::messenger;

/* Serialisation, communication and deserialisation of a controller capabilities object */
void controller_capabilities_exchange() {
  BOOST_MESSAGE(" Communicating empty controller_capabilities object ... ");

  controller_communicator cc;
  tool_communicator       tc;

  std::ostringstream      check_stream0;
  std::ostringstream      check_stream1;

  /** Initiative is on the side of the tool */
  tc.connect(cc);

  cc.get_capabilities().to_xml(check_stream0);

  tc.request_controller_capabilities();
  tc.get_controller_capabilities().to_xml(check_stream1);

  tc.disconnect();

  BOOST_CHECK(check_stream0.str() == check_stream1.str());

  BOOST_MESSAGE("  done");
  BOOST_MESSAGE(" Communicating filled controller_capabilities object ... ");

  check_stream0.str("");
  check_stream1.str("");

  /* Example display dimensions */
  controller_capabilities capabilities();

  cc.set_display_dimensions(75,75,0);

  tc.connect(cc);

  tc.request_controller_capabilities();
  cc.get_capabilities().to_xml(check_stream0);
  tc.get_controller_capabilities().to_xml(check_stream1);

  tc.disconnect();

  BOOST_CHECK(check_stream0.str() == check_stream1.str());

  BOOST_MESSAGE("  done");
}

/* Serialisation, communication and deserialisation of an input configuration */
void input_configuration_exchange() {
  BOOST_MESSAGE(" Communicating input configuration ... ");

  controller_communicator cc;
  tool_communicator       tc;

  std::ostringstream      check_stream0;
  std::ostringstream      check_stream1;

  tc.add_input_configuration("Testing", "aut");
  tc.add_input_configuration("Testing", "svc");

  tc.connect(cc);

  tc.disconnect();

  BOOST_CHECK(check_stream0.str() == check_stream1.str());
  BOOST_MESSAGE("  done");
}

/* Serialisation, communication and deserialisation of a report object */
void report_exchange() {
  BOOST_MESSAGE(" Communicating empty report ... ");

  std::ostringstream temporary;

  sip_messenger    c;
  sip_messenger    d;
  report           r;
  sip_message      m;

  r.to_xml(temporary);

  m.set_content(temporary.str());

  d.add_listener();
  c.connect();

  c.send_message(m);

  d.await_message(sip::unknown);

  BOOST_CHECK(d.pop_message().to_string() == std::string("<report></report>"));

  BOOST_MESSAGE("  done");
  BOOST_MESSAGE(" Communicating filled report ... ");

  /* New tool configuration */
  configuration* config = new configuration;

  /* An option for specifying an input file */
  config->add_option(std::string("Input file 0"));
  config->add_option(std::string("-r"));
  config->add_option(std::string("-s"));

  option::option_ptr t = config->get_option("Input file 0");

  /* The option has a URI as argument */
//  t.append_argument(sip::datatype::uri, std::string("/bin/bash"));
  t->append_argument(sip::datatype::standard_string, std::string("/bin/bash"));
//  something.append_type(sip::datatype::integer);
//  something.append_type(sip::datatype::real);
//  something.append_type(sip::datatype::enumeration);

  /* Add the option to the configuration */

  r.set_error("Everything okay!");
  r.set_comment("Lookin' good!");
  r.add_output("/etc/passwd", "text/plain");
  r.set_configuration(config);

  /* Serialise r */
  temporary.str(std::string(""));
  r.to_xml(temporary);

  m.set_content(temporary.str());

  c.send_message(m);

  d.await_message(sip::unknown);

  std::ostringstream copy;

std::cerr << " Reconstruction from XML stream ..." << std::endl;
std::cerr << "  Message  `" << d.peek_message().to_string() << "'" << std::endl;
  temporary.str(std::string(""));
  r.to_xml(temporary);

  /* New reader */
  xml2pp::text_reader reader(d.peek_message().to_string());

  report::from_xml(reader)->to_xml(copy);
std::cerr << "  Message  `" << copy.str() << "'" << std::endl;

  BOOST_CHECK(copy.str() == temporary.str());

  c.disconnect(d);

  BOOST_MESSAGE("  done");
}

void display_layout_exchange() {
  BOOST_MESSAGE(" Communicating input configuration ... ");

  controller_communicator cc;
  tool_communicator       tc;

  std::ostringstream      check_stream0;
  std::ostringstream      check_stream1;

  tc.connect(cc);

//  tc.send_display_layout();

  tc.disconnect();
  BOOST_MESSAGE("  done");
}

void command_line_interface_helper(int bc, int ac, char* const * const ba, const char* const * const aa, std::string m) {
  char* copy[bc];

  BOOST_MESSAGE("  " + m + "...");

  for (int i = 0; i < bc; ++i) {
    copy[i] = ba[i];
  }

  sip::cli::command_line_argument_extractor t(bc, copy);

  for (int i = 0; i < ac; ++i) {
    BOOST_CHECK(strcmp(copy[i], aa[i]) == 0);
  }

  BOOST_MESSAGE("   done");
}

void command_line_interface() {
  char* arguments[6] = { "-i", "filename", "--si-connect=socket://localhost:55923", "--a", "filename", "--si-connect=traditional" };
  char* reference[4] = { "-i", "filename", "--a", "filename" };

  BOOST_MESSAGE(" Command line argument extraction... ");

  command_line_interface_helper(5, 4, arguments, reference, "Trying socket scheme");

  arguments[2] = "--si-connect=traditional";

  command_line_interface_helper(5, 4, arguments, reference, "Trying traditional scheme");

  command_line_interface_helper(6, 4, arguments, reference, "Multiple connect specifications");
}

test_suite* init_unit_test_suite(int argc, char * argv[]) {
  using namespace boost;
  using namespace boost::unit_test;

  test_suite* test = BOOST_TEST_SUITE( "SI protocol implementation" );

  /* Change log parameters */
  unit_test_log_t::instance().set_threshold_level(log_messages);

  test->add(BOOST_TEST_CASE(&controller_capabilities_exchange), 0, 2);
  test->add(BOOST_TEST_CASE(&input_configuration_exchange), 0, 2);
  test->add(BOOST_TEST_CASE(&report_exchange), 0, 2);
  test->add(BOOST_TEST_CASE(&command_line_interface), 0, 2);

  return (test);
}

