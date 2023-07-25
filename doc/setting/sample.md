# Sampling Parameters

The defaults are okay for chatting.

## Mirostat
Mirostat version 2 is the current default with the following parameters.

```lisp
(temperature 0.7)
; Use Mirostat version 2.
(mirostat 2)
; Target entropy.
(mirostat_tau 5.0)
; Learning rate.
(mirostat_eta 0.1)
```

## Top-K
```lisp
(temperature 0.7)
; Turn Mirostat off.
(mirostat 0)
(top_k 40)
(top_p 0.9)
; Tail Free Sampling cutoff (default is 1.0,, off)
(tfs_z 0.9)
; Locally Typical Sampling cutoff (default is 1.0, off)
(typical_p 0.9)
```

## Misc
```lisp
; Random seed (default is time-based, different every run).
(seed 1234)

; Number of threads to use (default is 1).
; Can be changed later via the `/thread_count 8` command.
(thread_count 8)

; Batch size (default is 512, large enough to make OpenBLAS useful).
; Can be changed later via the `/batch_count 8` command.
(batch_count 512)
```
