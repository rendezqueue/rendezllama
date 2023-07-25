# Penalizing Tokens

The defaults are okay for chatting.

## Repeat penalty

```lisp
; Penalizes the most recent 20 tokens from being generated.
(repeat_window 256)
; How much to penalize repeated tokens.
(repeat_penalty 1.17647)
```

There are other options to penalize repetition.
```lisp
; Frequency penalty (default off, 0.0).
(frequency_penalty 0.1)
; Presence penalty (default off, 0.0).
(presence_penalty 0.1)
```

