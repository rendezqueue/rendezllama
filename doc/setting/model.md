# Model Loading

## File
I prefer using flags to specify model files.
- `--model ggml-model-q4_0.bin` are the model weights. Usually quantized.
  - Required.
- `--lora ggml-adapter-model.bin` gives a LoRA.
- `--lora_base ggml-model-f16.bin` gives higher-precision model weights to apply the LoRA on top of.
  - Not required when using `--lora` but you'll otherwise get a warning if the `--model` weights are low-precision.

Even though the flags are preferred, `setting.sxproto` supports them too:
```lisp
(model "ggml-model-q4_0.bin")
(lora "ggml-adapter-model.bin")
(lora_base "ggml-model-f16.bin")
```

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

These memory options are supported as flags too.

