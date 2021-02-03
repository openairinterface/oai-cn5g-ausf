#ifndef _PRINT_BUFFER_H
#define _PRINT_BUFFER_H

#include "iostream"
#include "logger.hpp"
#include <string>
using namespace std;

void print_buffer(const string app, const string commit, uint8_t *buf,
                  int len) {
  if (!app.compare("ausf_server"))
    cout << commit.c_str() << endl;
  Logger::ausf_server().debug(commit.c_str());

  for (int i = 0; i < len; i++)
    printf("%x ", buf[i]);
  printf("\n");
}

void hexStr2Byte(const char *src, unsigned char *dest, int len) {
  short i;
  unsigned char hBy, lBy;
  for (i = 0; i < len; i += 2) {
    hBy = toupper(src[i]);
    lBy = toupper(src[i + 1]);
    if (hBy > 0x39)
      hBy -= 0x37;
    else
      hBy -= 0x30;
    if (lBy > 0x39)
      lBy -= 0x37;
    else
      lBy -= 0x30;
    dest[i / 2] = (hBy << 4) | lBy;
  }
}

#endif
