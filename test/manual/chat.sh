#!/bin/sh
set -e -u

# Try running from the project's toplevel directory as:
#  ./test/manual/chat.sh roshambo_kira -- --model ../llama.cpp/models/7B/ggml-model-q4_0.gguf

# These will be overridden by given args.
model_file="../llama.cpp/models/7B/ggml-model-q4_0.gguf"
thread_count="8"

# First arg must be the test prompt name.
prompt_name="${1:-}"
shift
if [ -z "${prompt_name}" ]; then
  echo "Give a prompt name (a subdirectory in example/prompt/)." 1>&2
  exit 64
fi
setting_file="example/prompt/${prompt_name}/setting.sxpb"
transcript_file="bld/example/prompt/${prompt_name}.txt"

# Second arg can be "--" to indicate that we're forarding the rest.
if [ "--" = "${1:-}" ]; then
  shift
fi

exec ./bld/src/chat/chat \
  --x_setting "${setting_file}" \
  --o_rolling "${transcript_file}" \
  --thread_count "${thread_count}" \
  --model "${model_file}" \
  "$@"

