# Cortex-M0 Proof of Portability

`python3 tools/scripts/cortex_m0_proof.py` compiles the architecture-independent
list, scheduler, wait, timeout, public types, and a compile probe with:

```text
--target=arm-none-eabi -mcpu=cortex-m0 -mthumb
```

This proves that those C modules do not require Cortex-M3 instructions or
STM32F1 headers. It is deliberately not presented as a complete Cortex-M0
runtime port: Cortex-M0 needs different context-switch assembly because high
register save/restore instructions differ, and it still needs a board startup,
linker script, timer, and hardware validation.
