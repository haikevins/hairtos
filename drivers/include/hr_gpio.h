#ifndef HR_GPIO_H
#define HR_GPIO_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    HR_GPIO_PORT_A = 0,
    HR_GPIO_PORT_B,
    HR_GPIO_PORT_C
} hr_gpio_port_t;

typedef enum
{
    HR_GPIO_SPEED_2MHZ = 2,
    HR_GPIO_SPEED_10MHZ = 1,
    HR_GPIO_SPEED_50MHZ = 3
} hr_gpio_speed_t;

void hr_gpio_enable_port_clock(hr_gpio_port_t port);
void hr_gpio_config_output_push_pull(hr_gpio_port_t port,
                                     uint32_t pin,
                                     hr_gpio_speed_t speed);
void hr_gpio_config_alternate_push_pull(hr_gpio_port_t port,
                                        uint32_t pin,
                                        hr_gpio_speed_t speed);
void hr_gpio_config_input_floating(hr_gpio_port_t port, uint32_t pin);
void hr_gpio_write(hr_gpio_port_t port, uint32_t pin, bool high);
bool hr_gpio_read(hr_gpio_port_t port, uint32_t pin);
void hr_gpio_toggle(hr_gpio_port_t port, uint32_t pin);

#endif /* HR_GPIO_H */
