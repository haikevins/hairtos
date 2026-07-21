BOARD_SOURCES += \
    boards/bluepill_f103c8/board.c \
    boards/bluepill_f103c8/board_clock.c

BOARD_INCLUDES += -Iboards/bluepill_f103c8/include
BOARD_LINKER_SCRIPT := boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld
