#include "board.h"

int main(void)
{
    uint32_t heartbeat = 0UL;

    board_init();
    board_uart_write_line("Bare-metal foundation ready.");
    board_uart_write_line("LED PC13 toggles every 500 ms.");

    for (;;)
    {
        board_led_toggle();
        heartbeat++;

        board_uart_write_string("heartbeat=");
        board_uart_write_u32(heartbeat);
        board_uart_write_string(" uptime_ms=");
        board_uart_write_u32(board_millis());
        board_uart_write_line("");

        board_delay_ms(500UL);
    }
}
