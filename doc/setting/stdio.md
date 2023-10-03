# Standard Input and Output

## Limit
Token generated can be limited to sentence boundaries.
These options can be set via commands or in `setting.sxpb` as:
```lisp
; Limit number of sentences to 10 (default is 0, unlimited).
(sentence_limit 10)
; Limit number of tokens per sentence 100 (default is 0, unlimited).
(sentence_token_limit 100)
; Tokens that mark the end of each sentence (default is shown).
((sentence_terminals) "." "!" "?" "…")
```

## Coprocess Mode
When run as a coprocess, the expects to be controlled via commands like `/puts`, `/gets`, and `/d` (see [assistant_coprocess example](example/prompt/assistant_coprocess/)).

```lisp
; Run as a coprocess (default off).
; The program will only write to stdout when requested.
; Also available as a `--coprocess_mode_on 1` flag.
(coprocess_mode_on 1)
```
