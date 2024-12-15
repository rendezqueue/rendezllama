#ifndef RENDEZLLAMA_LANGUAGE_INFERENCE_SCHEMA_HH_
#define RENDEZLLAMA_LANGUAGE_INFERENCE_SCHEMA_HH_

#include <optional>
#include <vector>
#include <variant>

#include <fildesh/sxproto.h>

namespace rendezllama {
namespace inference {

struct Dry {
  float multiplier = 0.0;
  float base = 0.0;
  unsigned allowed_length = 0;
  unsigned window_length = 0;
};

struct PenalizeWith {
  unsigned window_length = 0;
  float repetition = 1.0;
  float frequency = 0.0;
  float presence = 0.0;
};

struct Xtc {
  float probability = 0.0;
  float threshold = 0.1;
};

enum AdjustViaType {
  AdjustViaType_NULL,
  AdjustViaType_Dry,
  AdjustViaType_PenalizeWith,
  AdjustViaType_top_k,
  AdjustViaType_tfs_z,
  AdjustViaType_typical_p,
  AdjustViaType_top_p,
  AdjustViaType_min_p,
  AdjustViaType_temperature,
  AdjustViaType_Xtc,
};


typedef std::variant<
  std::monostate,
  Dry,
  PenalizeWith,
  unsigned,  // top_k
  float,  // tfs_z
  float,  // typical_p
  float,  // top_p
  float,  // min_p
  float,  // temperature
  Xtc
> AdjustVia;

struct Mirostat {
  unsigned version = 1;
  float tau = 5.0;
  float eta = 0.1;
};

struct Probability {};

typedef std::variant<
  std::monostate,
  Mirostat,
  Probability
> PickVia;

struct Sampling {
  std::vector<AdjustVia> adjust_thru;
  PickVia pick_via;
  unsigned seed = 0;
};

typedef std::variant<
  std::monostate,
  Sampling
> InferVia;

struct Language {
  InferVia infer_via;
};

bool
populate_AdjustVia(
    AdjustVia& adjust_via,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it);
bool
populate_PickVia(
    PickVia& pick_via,
    const FildeshSxpb* sxpb,
    FildeshSxpbIT it);

}  // namespace inference

const FildeshSxprotoField* language_sxproto_schema();

}  // namespace rendezllama
#endif
