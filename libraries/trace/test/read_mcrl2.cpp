#include <boost/test/minimal.hpp>
#include <exception>
#include <sstream>
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/trace/trace.h"
#include "mcrl2/core/print.h"

using namespace std;
using namespace mcrl2::trace;

// data generated from data/abc.mcrl2


static char trace_data[] =
"\x6d\x43\x52\x4c\x32\x54\x72\x61\x63\x65\x01\x00\x00\x8b\xaf\x83"
"\x00\x15\x3b\x05\x5b\x5f\x2c\x5f\x5d\x02\x00\x17\x06\x14\x00\x0a"
"\x06\x10\x0b\x02\x00\x01\x02\x5b\x5d\x00\x00\x01\x05\x41\x63\x74"
"\x49\x64\x02\x00\x03\x03\x04\x05\x03\x02\x00\x01\x01\x61\x00\x01"
"\x01\x01\x63\x00\x01\x01\x01\x62\x00\x01\x01\x06\x53\x6f\x72\x74"
"\x49\x64\x01\x00\x03\x03\x07\x08\x09\x03\x4e\x61\x74\x00\x01\x01"
"\x04\x42\x6f\x6f\x6c\x00\x01\x01\x03\x50\x6f\x73\x00\x01\x01\x08"
"\x44\x61\x74\x61\x41\x70\x70\x6c\x02\x00\x04\x01\x0b\x01\x00\x04"
"\x4f\x70\x49\x64\x02\x00\x05\x05\x0e\x13\x12\x0d\x0f\x02\x0c\x06"
"\x09\x53\x6f\x72\x74\x41\x72\x72\x6f\x77\x02\x00\x02\x01\x00\x01"
"\x06\x05\x40\x63\x44\x75\x62\x00\x01\x01\x04\x74\x72\x75\x65\x00"
"\x01\x01\x03\x40\x63\x31\x00\x01\x01\x06\x41\x63\x74\x69\x6f\x6e"
"\x02\x00\x03\x01\x02\x02\x00\x01\x03\x4e\x69\x6c\x00\x00\x01\x05"
"\x66\x61\x6c\x73\x65\x00\x01\x01\x05\x40\x63\x4e\x61\x74\x00\x01"
"\x01\x04\x70\x61\x69\x72\x02\x00\x03\x01\x00\x01\x11\x00\x80\xa0"
"\xc3\x15\x04\x08\x35\x05\x94\x42\x60\xc6\xa1\x18\x82\x54\xa8\x04"
"\x64\x4a\x35\x1c\xc8\x33\x51\x25\x34\x24\x20\x05\x48\xaa\x13\x0c"
"\x94\xd5\x20\x22\x21\xb1\x52\xa0\x33\xcb\x47\x54\x52\x80";

static size_t trace_data_size = sizeof(trace_data);

static bool read_trace(Trace& t, char* buf, size_t size)
{
  stringstream is(ios_base::in | ios_base::out | ios_base::binary);
  is.write(buf,size);

  try
  {
    t.load(is,tfMcrl2);
  }
  catch (runtime_error e)
  {
    BOOST_ERROR(e.what());
    return false;
  }

  return true;
}

void test_next_action(Trace& t, const char* s)
{
  mcrl2::lps::multi_action a = t.currentAction();
  t.increasePosition();
  BOOST_CHECK((a != mcrl2::lps::multi_action()));
  if (a != mcrl2::lps::multi_action())
  {
    string action(pp(a));
    BOOST_CHECK(action == s);
  }
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc,argv);

  Trace t;

  BOOST_REQUIRE(read_trace(t,trace_data,trace_data_size));

  BOOST_REQUIRE(t.number_of_actions() == 3);

  test_next_action(t,"a");
  test_next_action(t,"b(1, true)");
  test_next_action(t,"c");

  return 0;
}
