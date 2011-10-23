#include <boost/test/minimal.hpp>
#include <exception>
#include <sstream>
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/trace/trace.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/detail/pp_deprecated.h"

using namespace std;
using namespace mcrl2::trace;

// data generated from data/abc.mcrl2
static char trace_data[] = "\x6d\x43\x52\x4c\x32\x54\x72\x61\x63\x65\x01\x00\x00\x8b\xaf\x83\x00\x15\x3a\x05\x5b\x5f\x2c\x5f\x5d\x02\x00\x15\x06\x14\x13\x0c\x05\x02\x12\x02\x00\x01\x02\x5b\x5d\x00\x00\x01\x06\x53\x6f\x72\x74\x49\x64\x01\x00\x02\x02\x03\x04\x04\x42\x6f\x6f\x6c\x00\x01\x01\x03\x50\x6f\x73\x00\x01\x01\x04\x4f\x70\x49\x64\x02\x00\x04\x04\x06\x07\x09\x08\x02\x0a\x02\x04\x74\x72\x75\x65\x00\x01\x01\x05\x66\x61\x6c\x73\x65\x00\x01\x01\x03\x40\x63\x31\x00\x01\x01\x05\x40\x63\x44\x75\x62\x00\x01\x01\x09\x53\x6f\x72\x74\x41\x72\x72\x6f\x77\x02\x00\x01\x01\x00\x01\x02\x03\x4e\x69\x6c\x00\x00\x01\x08\x44\x61\x74\x61\x41\x70\x70\x6c\x02\x00\x03\x01\x05\x01\x00\x01\x62\x00\x01\x01\x01\x63\x00\x01\x01\x05\x41\x63\x74\x49\x64\x02\x00\x03\x03\x0e\x0d\x10\x02\x00\x01\x01\x61\x00\x01\x01\x07\x4d\x75\x6c\x74\x41\x63\x74\x01\x00\x03\x01\x00\x06\x41\x63\x74\x69\x6f\x6e\x02\x00\x03\x01\x0f\x02\x00\x01\x05\x53\x54\x41\x54\x45\x01\x00\x04\x02\x0c\x05\x04\x70\x61\x69\x72\x02\x00\x03\x01\x11\x01\x0b\x00\x82\x1a\x23\x20\x01\x41\xa8\x4c\x81\x10\x83\x04\x09\x04\xca\x51\x8c\x22\x22\xa5\x6a\x18\x42\x86\x81\x60\x73\x61\x54\x06\x22\x8a\xd8\xc1\xe0\xba\xaa\x54\x3a\x61\x96\xc8\x32\x28";
static size_t trace_data_size = sizeof(trace_data);

static bool read_trace(Trace& t, char* buf, size_t size)
{
  stringstream is(ios_base::in | ios_base::out | ios_base::binary);
  is.write(buf,size);

  try
  {
    t.load(is);
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
  ATerm a = (ATerm) t.nextAction();
  BOOST_CHECK((a != NULL) && (ATgetType(a) == AT_APPL));
  if ((a != NULL) && (ATgetType(a) == AT_APPL))
  {
    string action(mcrl2::core::pp_deprecated(a));
    BOOST_CHECK(action == s);
  }
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc,argv);

  Trace t;

  BOOST_REQUIRE(read_trace(t,trace_data,trace_data_size));

  BOOST_REQUIRE(t.getLength() == 3);

  test_next_action(t,"a");
  test_next_action(t,"b(1, true)");
  test_next_action(t,"c");

  return 0;
}
