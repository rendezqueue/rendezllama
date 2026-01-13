#include <cassert>

#include <fildesh/fildesh.h>

#include "src/chat/opt.hh"

int main(int argc, char** argv)
{
  bool good = true;
  assert(argc == 2);
  const char* filename = argv[1];
  rendezllama::ChatOptions opt;
  FildeshX* in = open_FildeshXF(filename);
  assert(in);
  good = rendezllama::slurp_sxpb_initialize_options_close_FildeshX(
      in, opt, filename);
  assert(good);
  return (good ? 0 : 1);
}
