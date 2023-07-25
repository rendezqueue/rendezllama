#include <cassert>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"

int main(int argc, char** argv)
{
  assert(argc == 2);
  const char* filename = argv[1];
  rendezllama::ChatOptions opt;
  int exstatus = rendezllama::parse_initialize_options_sxproto(opt, filename);
  assert(exstatus == 0);
  return 0;
}
