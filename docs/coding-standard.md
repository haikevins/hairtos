# Coding Standard

Use C11 and GNU Assembly only in the architecture port. Kernel public names use
`hr_`; HairEvent names use `he_`. Configuration macros use `HR_CFG_` or
`HE_CFG_`.

Use four spaces, Allman braces, a 100-column target, fixed-width integer types,
header guards, and `static` for private functions.

No recursion, variable-length arrays, hidden heap allocation, ignored status
without explanation, callbacks inside critical sections, or undocumented casts.

Comments explain invariants, ownership, concurrency, hardware reasons, and
tradeoffs rather than restating code.
