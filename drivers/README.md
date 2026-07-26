# Drivers

## Layout

```text
drivers/
├── include/       # SoC-independent driver interfaces
├── common/        # Reusable implementation shared by compatible targets
└── stm32f1/       # STM32F1 register-level implementations
```

Application and board code include headers only from `drivers/include`.
A new SoC must add a separate implementation directory such as
`drivers/stm32f4/` without changing the public headers unless the abstraction
itself must evolve.

The current drivers are intentionally small and polling-oriented: GPIO, UART
and a millisecond hardware-timer interface. They do not depend on kernel
internal headers.
