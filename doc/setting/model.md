# Model Loading

## File
I prefer using flags to specify model files.
- `--model ggml-model-q4_0.gguf` are the model weights. Usually quantized.
  - Required.
- `--lora ggml-adapter-model.gguf` gives a LoRA.
- `--lora_base ggml-model-f16.gguf` gives higher-precision model weights to apply the LoRA on top of.
  - Not required when using `--lora` but you'll otherwise get a warning if the `--model` weights are low-precision.

Even though the flags are preferred, `setting.sxproto` supports them too:
```lisp
(model "ggml-model-q4_0.gguf")
(lora "ggml-adapter-model.gguf")
(lora_base "ggml-model-f16.gguf")
```

## Context
```lisp
; Set the model's default context limit as 4096 for Llama-2 (default comes from the model).
(model_token_limit 4096)
; Set the prompt's context limit as 5000 (default is model_token_limit).
(context_token_limit 5000)
```

The first option can be initialized via a flag like `--model_token_limit 4096`, which is also used as the default value for `context_token_limit`.

## Memory
By default, we use mmap to load the model.
This makes the system hold and manage the model data, loading it as needed or letting multiple programs read it without duplicating it in memory.

This can introduce a bottleneck when low-priority stuff (like ZFS disk cache) is preventing the mmapped model from staying in RAM.
In that case, you can try focing the model into memory with mlock:
```lisp
; Tries to lock the model in memory (default off, 0).
(mlock_on 1)
; If the above doesn't work...
; Load model into program memory by disabling mmap (default on, 1).
(mmap_on 0)
```

These memory options are also supported as `--mlock_on 1` and `--mmap_on 0` flags.

## Compute
```lisp
; Number of threads to use (default is 1).
; Can be changed later via the `/thread_count 8` command.
(thread_count 8)
; Warning: This number should exclude hyperthreads.

; Batch size (default is 512, large enough to make OpenBLAS useful).
; Can be changed later via the `/batch_count 8` command.
(batch_count 512)
; Warning: Setting this too large (e.g., 2048) can cause assertion violations.
```

