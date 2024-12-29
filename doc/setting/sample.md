# Sampling Parameters

The defaults are okay for chatting.

## Mirostat
Mirostat version 2 is the current default with the following parameters.

```lisp
(temperature 0.7)
(language
 ((infer_via sampling)
  ((pick_via mirostat)
   ; Use Mirostat version 2.
   (version 2)
   ; Target entropy.
   (tau 5.0)
   ; Learning rate.
   (eta 0.1)
  )
 )
)
```

## Probability
```lisp
(temperature 0.7)
; Top-K Sampling (default is 1000).
(top_k 40)
; Top-P Sampling (default is 0.95).
(top_p 0.9)
; Min-P Sampling (default is 0.05).
(min_p 0.05)
; Locally Typical Sampling cutoff (default is 1.0, off).
(typical_p 0.9)
(language
 ((infer_via sampling)
  ; Turn Mirostat off.
  ((pick_via probability))
 )
)
```

## Misc
```lisp
(language
 ((infer_via sampling)
  ; Random seed (default is time-based, different every run).
  (seed 1234)
 )
)
```
