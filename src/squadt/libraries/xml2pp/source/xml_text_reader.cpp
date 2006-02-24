#include <xml2pp/xml_text_reader.h>

namespace xml2pp {

  void text_reader::read() throw () {
    int status = xmlTextReaderRead(reader);

    do {
      if (status <= 0) {
        /* Process error, or end of file */
        throw (exception::error_while_parsing_document);
      }
      else {
        /* Skip white space */
        if (xmlTextReaderNodeType(reader) == XML_READER_TYPE_SIGNIFICANT_WHITESPACE) {
          /* Text is no profile, skip */
          status = xmlTextReaderRead(reader);

          continue;
        }
      }

      break;
    }
    while (1);
  }
}

