#!/bin/sh
set -e -u

# Try running from the project's toplevel directory as:
#  ./test/manual/chat.sh roshambo_0 -- --model ../llama.cpp/models/7B/ggml-model-q4_0.bin

# These will be overridden by given args.
model_file="../llama.cpp/models/7B/ggml-model-q4_0.bin"
thread_count="8"

# First arg must be the test prompt name.
prompt_name="${1:-}"
shift
if [ -z "${prompt_name}" ]; then
  echo "Give a prompt name (a subdirectory in test/prompt/)." 1>&2
  exit 64
fi
priming_file="test/prompt/${prompt_name}/priming.txt"
rolling_file="test/prompt/${prompt_name}/rolling.txt"
tee_file="test/prompt/${prompt_name}/tee.txt"

# Second arg can be "--" to indicate that we're forarding the rest.
if [ "--" = "${1:-}" ]; then
  shift
fi

./bld/src/chat/chat \
  --x_priming "${priming_file}" --x_rolling "${rolling_file}" \
  --model "${model_file}" \
  --thread_count "${thread_count}" \
  --context_token_limit "2048" \
  --sentence_token_limit 32 \
  "$@" |
tee "${tee_file}"

