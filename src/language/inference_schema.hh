#ifndef RENDEZLLAMA_LANGUAGE_INFERENCE_SCHEMA_HH_
#define RENDEZLLAMA_LANGUAGE_INFERENCE_SCHEMA_HH_

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
  float repetition = 1.0f;
  float frequency = 0.0f;
  float presence = 0.0f;
};

struct Xtc {
  float threshold = 0.15f;
  float probability = 1.0f;
};

struct AdjustViaKind {
  enum E : std::size_t {
    none,
    dry,
    min_p,
    penalize_with,
    temperature,
    top_k,
    top_p,
    typical_p,
    xtc,
  };
};

typedef std::variant<
  std::monostate,
  Dry,
  float,  // min_p
  PenalizeWith,
  float,  // temperature
  unsigned,  // top_k
  float,  // top_p
  float,  // typical_p
  Xtc
> AdjustVia;

struct AdaptiveP {
  float target = -1.0f;
  float decay = 0.9f;
};

struct Determinism {};

struct Mirostat {
  unsigned version = 2;
  float tau = 5.0f;
  float eta = 0.1f;
};

struct Probability {};

typedef std::variant<
  std::monostate,
  AdaptiveP,
  Determinism,
  Mirostat,
  Probability
> PickVia;

struct Sampling {
  int seed = -1;
  std::vector<AdjustVia> adjust_thru;
  PickVia pick_via;
};

typedef std::variant<
  std::monostate,
  Sampling
> InferVia;

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
bool
populate_InferVia(
    InferVia& infer_via,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it);

}  // namespace inference

const FildeshSxprotoField* inference_sxproto_schema();

}  // namespace rendezllama
#endif
