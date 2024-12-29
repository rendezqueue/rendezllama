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
  float probability = 0.0f;
  float threshold = 0.1f;
};


struct AdjustViaKind {
  enum E : std::size_t {
    none,
    dry,
    penalize_with,
    top_k,
    tfs_z,
    typical_p,
    top_p,
    min_p,
    temperature,
    xtc,
  };
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
  unsigned version = 2;
  float tau = 5.0f;
  float eta = 0.1f;
};

struct Probability {};

typedef std::variant<
  std::monostate,
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
