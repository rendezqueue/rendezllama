#include "src/language/inference_schema.hh"

#include <cstdio>
#include <string_view>

using rendezllama::inference::AdjustViaKind;

static FildeshSxprotoField penalize_with_fields[] = {
  {"window_length", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
  {"repetition", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"frequency", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"presence", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField xtc_fields[] = {
  {"probability", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"threshold", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField dry_fields[] = {
  {"multiplier", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"base", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"allowed_length", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
  {"window_length", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
};

static FildeshSxprotoField adjust_thru_manyof[] = {
  {"dry", FILL_FildeshSxprotoField_MESSAGE(dry_fields)},
  {"penalize_with", FILL_FildeshSxprotoField_MESSAGE(penalize_with_fields)},
  {"top_k", FILL_FildeshSxprotoField_INT(1, INT_MAX)},
  {"tfs_z", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"typical_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"top_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"min_p", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"temperature", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"xtc", FILL_FildeshSxprotoField_MESSAGE(xtc_fields)},
};

static FildeshSxprotoField mirostat_fields[] = {
  {"version", FILL_FildeshSxprotoField_INT(1, 2)},
  {"tau", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
  {"eta", FILL_DEFAULT_FildeshSxprotoField_FLOAT},
};

static FildeshSxprotoField probability_fields[] = {
  {"none", FILL_DEFAULT_FildeshSxprotoField_STRING},
};

static FildeshSxprotoField pick_via_oneof[] = {
  {"mirostat", FILL_FildeshSxprotoField_MESSAGE(mirostat_fields)},
  {"probability", FILL_FildeshSxprotoField_MESSAGE(probability_fields)},
};

static FildeshSxprotoField sampling_fields[] = {
  {"seed", FILL_FildeshSxprotoField_INT(0, INT_MAX)},
  {"adjust_thru", FILL_FildeshSxprotoField_MANYOF(adjust_thru_manyof)},
  {"pick_via", FILL_FildeshSxprotoField_LONEOF(pick_via_oneof)},
};

static FildeshSxprotoField infer_via_oneof[] = {
  {"sampling", FILL_FildeshSxprotoField_MESSAGE(sampling_fields)},
};

const FildeshSxprotoField* rendezllama::inference_sxproto_schema() {
  DECLARE_TOPLEVEL_FildeshSxprotoField(schema, infer_via_oneof);
  if (!schema->name) {
    lone_toplevel_initialization_FildeshSxprotoField(schema);
    schema->kind = FildeshSxprotoFieldKind_LONEOF;
  }
  return schema;
}

  bool
rendezllama::inference::populate_AdjustVia(
    rendezllama::inference::AdjustVia& adjust_via,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it)
{
  const std::string_view name = name_at_FildeshSxpb(sxpb, it);
  if (name == "min_p") {
    adjust_via.emplace<AdjustViaKind::min_p>(
        float_value_at_FildeshSxpb(sxpb, it));
  }
  else if (name == "top_k") {
    adjust_via.emplace<AdjustViaKind::top_k>(
        unsigned_value_at_FildeshSxpb(sxpb, it));
  }
  else if (name == "top_p") {
    adjust_via.emplace<AdjustViaKind::top_p>(
        float_value_at_FildeshSxpb(sxpb, it));
  }
  else if (name == "typical_p") {
    adjust_via.emplace<AdjustViaKind::typical_p>(
        float_value_at_FildeshSxpb(sxpb, it));
  }
  else if (name == "temperature") {
    adjust_via.emplace<AdjustViaKind::temperature>(
        float_value_at_FildeshSxpb(sxpb, it));
  }
  else if (name == "xtc") {
    rendezllama::inference::Xtc xtc;
    lone_subfield_at_FildeshSxpb_to_float(&xtc.probability, sxpb, it, "probability");
    lone_subfield_at_FildeshSxpb_to_float(&xtc.threshold, sxpb, it, "threshold");
    adjust_via.emplace<AdjustViaKind::xtc>(xtc);
  }
  else if (name == "dry") {
    rendezllama::inference::Dry dry;
    lone_subfield_at_FildeshSxpb_to_float(&dry.multiplier, sxpb, it, "multiplier");
    lone_subfield_at_FildeshSxpb_to_float(&dry.base, sxpb, it, "base");
    lone_subfield_at_FildeshSxpb_to_unsigned(&dry.allowed_length, sxpb, it, "allowed_length");
    lone_subfield_at_FildeshSxpb_to_unsigned(&dry.window_length, sxpb, it, "window_length");
    adjust_via.emplace<AdjustViaKind::dry>(dry);
  }
  else if (name == "penalize_with") {
    rendezllama::inference::PenalizeWith penalize_with;
    lone_subfield_at_FildeshSxpb_to_float(&penalize_with.frequency, sxpb, it, "frequency");
    lone_subfield_at_FildeshSxpb_to_float(&penalize_with.presence, sxpb, it, "presence");
    lone_subfield_at_FildeshSxpb_to_float(&penalize_with.repetition, sxpb, it, "repetition");
    lone_subfield_at_FildeshSxpb_to_unsigned(&penalize_with.window_length, sxpb, it, "window_length");
    adjust_via.emplace<AdjustViaKind::penalize_with>(penalize_with);
  }
  else {
    return false;
  }
  return true;
}

  bool
rendezllama::inference::populate_PickVia(
    rendezllama::inference::PickVia& pick_via,
    const FildeshSxpb* sxpb,
    FildeshSxpbIT it)
{
  const FildeshSxpbIT mirostat_it = lookup_subfield_at_FildeshSxpb(sxpb, it, "mirostat");
  if (!nullish_FildeshSxpbIT(mirostat_it)) {
    rendezllama::inference::Mirostat mirostat;
    if (!lone_subfield_at_FildeshSxpb_to_unsigned(&mirostat.version, sxpb, mirostat_it, "version")) {
      mirostat.version = 2;
    }
    lone_subfield_at_FildeshSxpb_to_float(&mirostat.tau, sxpb, mirostat_it, "tau");
    lone_subfield_at_FildeshSxpb_to_float(&mirostat.eta, sxpb, mirostat_it, "eta");
    pick_via = mirostat;
    return true;
  }
  else {
    rendezllama::inference::Probability probability;
    pick_via = probability;
    return true;
  }
  return false;
}

  bool
rendezllama::inference::populate_InferVia(
    InferVia& infer_via,
    FildeshSxpb* sxpb,
    FildeshSxpbIT it)
{
  if (nullish_FildeshSxpbIT(it)) {
    return false;
  }
  const FildeshSxpbIT sampling_it = lookup_subfield_at_FildeshSxpb(sxpb, it, "sampling");
  Sampling sampling;
  if (!nullish_FildeshSxpbIT(sampling_it)) {
    unsigned seed = 0;
    if (lone_subfield_at_FildeshSxpb_to_unsigned(&seed, sxpb, sampling_it, "seed")) {
      sampling.seed = static_cast<int>(INT_MAX & seed);
    }

    it = lookup_subfield_at_FildeshSxpb(sxpb, sampling_it, "adjust_thru");
    for (it = first_at_FildeshSxpb(sxpb, it); !nullish_FildeshSxpbIT(it);
         it = next_at_FildeshSxpb(sxpb, it)) {
      AdjustVia adjust_via;
      if (populate_AdjustVia(adjust_via, sxpb, it)) {
        sampling.adjust_thru.push_back(adjust_via);
      }
    }

    FildeshSxpbIT pick_it = lookup_subfield_at_FildeshSxpb(sxpb, sampling_it, "pick_via");
    if (!nullish_FildeshSxpbIT(pick_it)) {
      populate_PickVia(sampling.pick_via, sxpb, pick_it);
    }
    else {
      Probability probability;
      sampling.pick_via = probability;
    }

    infer_via = sampling;
    return true;
  }
  return false;
}
