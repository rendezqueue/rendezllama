#include <cassert>

#include "llama.h"

#include "src/chat/trajectory.hh"

int main()
{
  assert(sizeof(llama_token) == sizeof(rendezllama::ChatTrajectory::Token_id));
  return 0;
}
