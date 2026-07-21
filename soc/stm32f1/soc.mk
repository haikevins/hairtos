SOC_SOURCES += \
    soc/stm32f1/system_stm32f1.c \
    soc/stm32f1/stm32f1_clock.c \
    soc/stm32f1/stm32f1_irq.c \
    soc/stm32f1/startup_stm32f103.S

SOC_INCLUDES += -Isoc/stm32f1/include
