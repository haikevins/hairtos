#include "hr_benchmark.h"

#include "hr_gpio.h"

#define HR_BENCHMARK_GPIO_PORT HR_GPIO_PORT_B
#define HR_BENCHMARK_GPIO_PIN  0U

void hr_benchmark_gpio_init(void)
{
    hr_gpio_enable_port_clock(HR_BENCHMARK_GPIO_PORT);
    hr_gpio_config_output_push_pull(HR_BENCHMARK_GPIO_PORT,
                                    HR_BENCHMARK_GPIO_PIN,
                                    HR_GPIO_SPEED_50MHZ);
    hr_gpio_write(HR_BENCHMARK_GPIO_PORT, HR_BENCHMARK_GPIO_PIN, false);
}

void hr_benchmark_gpio_mark_begin(void)
{
    hr_gpio_write(HR_BENCHMARK_GPIO_PORT, HR_BENCHMARK_GPIO_PIN, true);
}

void hr_benchmark_gpio_mark_end(void)
{
    hr_gpio_write(HR_BENCHMARK_GPIO_PORT, HR_BENCHMARK_GPIO_PIN, false);
}
