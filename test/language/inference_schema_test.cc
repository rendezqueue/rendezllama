#include "src/language/inference_schema.hh"

#include <cassert>

#include <fildesh/ostream.hh>

#include "src/chat/opt.hh"
#include "src/chat/opt_schema.hh"

using rendezllama::inference::AdjustViaKind;
using rendezllama::slurp_sxpb_dynamic_options_close_FildeshX;

static
  void
inference_parse_test()
{
  using rendezllama::inference::Mirostat;
  using rendezllama::inference::Probability;
  using rendezllama::inference::Sampling;

  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) ((pick_via mirostat))))");
  all_good = slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<Sampling>(opt.infer_via));
  auto& sampling = std::get<Sampling>(opt.infer_via);
  assert(std::holds_alternative<Mirostat>(sampling.pick_via));
  auto& mirostat = std::get<Mirostat>(sampling.pick_via);
  assert(mirostat.version == 2);

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) ((pick_via mirostat) (version 1))))");
  all_good = slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<Sampling>(opt.infer_via));
  sampling = std::get<Sampling>(opt.infer_via);
  assert(std::holds_alternative<Mirostat>(sampling.pick_via));
  mirostat = std::get<Mirostat>(sampling.pick_via);
  assert(mirostat.version == 1);

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) ((pick_via probability))))");
  all_good = slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<Sampling>(opt.infer_via));
  sampling = std::get<Sampling>(opt.infer_via);
  assert(std::holds_alternative<Probability>(sampling.pick_via));

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling)))");
  all_good = slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<Sampling>(opt.infer_via));
  sampling = std::get<Sampling>(opt.infer_via);
  assert(std::holds_alternative<Probability>(sampling.pick_via));
}

static
  void
seed_parse_test()
{
  using rendezllama::inference::Sampling;

  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (seed 123)))");
  all_good = slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<Sampling>(opt.infer_via));
  auto& sampling = std::get<Sampling>(opt.infer_via);
  assert(sampling.seed == 123);
}

static
  void
penalize_with_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (penalize_with (window_length 1000) (repetition 1.5) (frequency 0.5) (presence 0.25)))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* penalize_with = std::get_if<AdjustViaKind::penalize_with>(&sampling.adjust_thru[0]);
  assert(penalize_with);
  assert(penalize_with->window_length == 1000);
  assert(penalize_with->repetition == 1.5);
  assert(penalize_with->frequency == 0.5);
  assert(penalize_with->presence == 0.25);
}

static
  void
dry_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (dry (multiplier 0.5) (base 0.25) (allowed_length 100) (window_length 1000)))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* dry = std::get_if<AdjustViaKind::dry>(&sampling.adjust_thru[0]);
  assert(dry);
  assert(dry->multiplier == 0.5);
  assert(dry->base == 0.25);
  assert(dry->allowed_length == 100);
  assert(dry->window_length == 1000);
}

static
  void
xtc_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (xtc (probability 0.75) (threshold 0.25)))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* xtc = std::get_if<AdjustViaKind::xtc>(&sampling.adjust_thru[0]);
  assert(xtc);
  assert(xtc->probability == 0.75);
  assert(xtc->threshold == 0.25);
}

static
  void
min_p_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (min_p 0.25))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* min_p = std::get_if<rendezllama::inference::AdjustViaKind::min_p>(&sampling.adjust_thru[0]);
  assert(min_p);
  assert(*min_p == 0.25);
}

static
  void
top_k_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (top_k 123))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* top_k = std::get_if<rendezllama::inference::AdjustViaKind::top_k>(&sampling.adjust_thru[0]);
  assert(top_k);
  assert(*top_k == 123);
}

static
  void
top_p_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (top_p 0.75))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* top_p = std::get_if<rendezllama::inference::AdjustViaKind::top_p>(&sampling.adjust_thru[0]);
  assert(top_p);
  assert(*top_p == 0.75);
}

static
  void
typical_p_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (typical_p 0.5))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* typical_p = std::get_if<rendezllama::inference::AdjustViaKind::typical_p>(&sampling.adjust_thru[0]);
  assert(typical_p);
  assert(*typical_p == 0.5);
}

static
  void
temperature_parse_test()
{
  rendezllama::ChatOptions opt;
  FildeshX in[1];
  bool all_good;

  *in = FildeshX_of_strlit(
      "(language ((infer_via sampling) (adjust_thru (()) (temperature 0.75))))");
  all_good = rendezllama::slurp_sxpb_dynamic_options_close_FildeshX(in, opt);
  assert(all_good);
  assert(std::holds_alternative<rendezllama::inference::Sampling>(opt.infer_via));
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  assert(sampling.adjust_thru.size() == 1);
  auto* temperature = std::get_if<AdjustViaKind::temperature>(&sampling.adjust_thru[0]);
  assert(temperature);
  assert(*temperature == 0.75);
}


int main()
{
  inference_parse_test();
  seed_parse_test();
  penalize_with_parse_test();
  dry_parse_test();
  xtc_parse_test();
  min_p_parse_test();
  top_k_parse_test();
  top_p_parse_test();
  typical_p_parse_test();
  temperature_parse_test();
  return 0;
}
