#include "mime_parser.hpp"
#include <string>
#include <iostream>

bool multipart_parser(std::string input, std::string &jsonData, std::string &n1sm, std::string &n2sm) {

  //simple parser
  mime_parser sp = { };
  sp.parse(input);

  std::vector<mime_part> parts = { };
  sp.get_mime_parts(parts);
  uint8_t size = parts.size();
  //at least 2 parts for Json data and N1 (+ N2)
  if (size < 2) {
    return false;
  }

  jsonData = parts[0].body;
  n1sm = parts[1].body;
  bool is_ngap = false;
  if (size > 2) {
    n2sm = parts[2].body;
  } else {
    n2sm = "null";
  }

  return true;
}

