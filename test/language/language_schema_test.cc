#include "src/language/language_schema.hh"

#include <cassert>
#include <cstring>
#include <iostream>

#include <fildesh/ostream.hh>

#include "src/chat/opt.hh"
#include "src/chat/opt_schema.hh"

static
  void
substitution_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  *in = FildeshX_of_strlit(
      "(language (substitution (special_tokens (()) (() (name <|im_start|>)))))");
  bool all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  auto& substitution = opt.substitution;
  assert(substitution.special_tokens.size() == 1);
  assert(substitution.special_tokens[0].candidates.size() == 1);
  assert(substitution.special_tokens[0].alias == "<|im_start|>");
  assert(substitution.special_tokens[0].candidates[0] == "<|im_start|>");

  substitution.special_tokens.clear();
  *in = FildeshX_of_strlit(
      "(language\n"
      " (substitution\n"
      "  (bos_token_alias <bos_token>)\n"
      "  (eos_token_alias <eos_token>)\n"
      "  (special_tokens (())\n"
      "   (() (alias <|im_start|>) (candidates (()) <bos_token>))\n"
      "   (() (alias <|im_end|>) (candidates (()) <eos_token>))\n"
      ")))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(substitution.special_tokens.size() == 2);
  assert(substitution.special_tokens[0].candidates.size() == 1);
  assert(substitution.special_tokens[1].candidates.size() == 1);
  assert(substitution.special_tokens[0].alias == "<|im_start|>");
  assert(substitution.special_tokens[1].alias == "<|im_end|>");
  assert(substitution.special_tokens[0].candidates[0] == "<bos_token>");
  assert(substitution.special_tokens[1].candidates[0] == "<eos_token>");
}

int main()
{
  substitution_parse_test();
  return 0;
}
