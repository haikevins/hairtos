
#ifndef HR_GPIO_H
#define HR_GPIO_H

#include <stdbool.h>
#include <stdint.h>

/* Opaque target-defined pin identifier. */
typedef uint32_t hr_gpio_pin_t;

typedef enum
{
    HR_GPIO_MODE_INPUT = 0,
    HR_GPIO_MODE_OUTPUT_PUSH_PULL,
    HR_GPIO_MODE_OUTPUT_OPEN_DRAIN,
    HR_GPIO_MODE_ALTERNATE_PUSH_PULL,
    HR_GPIO_MODE_ALTERNATE_OPEN_DRAIN
} hr_gpio_mode_t;

typedef enum
{
    HR_GPIO_PULL_NONE = 0,
    HR_GPIO_PULL_UP,
    HR_GPIO_PULL_DOWN
} hr_gpio_pull_t;

typedef enum
{
    HR_GPIO_DRIVE_LOW = 0,
    HR_GPIO_DRIVE_MEDIUM,
    HR_GPIO_DRIVE_HIGH,
    HR_GPIO_DRIVE_VERY_HIGH
} hr_gpio_drive_t;

typedef struct
{
    hr_gpio_mode_t mode;
    hr_gpio_pull_t pull;
    hr_gpio_drive_t drive;
} hr_gpio_config_t;

bool hr_gpio_configure(hr_gpio_pin_t pin, const hr_gpio_config_t *config);
void hr_gpio_write(hr_gpio_pin_t pin, bool high);
bool hr_gpio_read(hr_gpio_pin_t pin);
void hr_gpio_toggle(hr_gpio_pin_t pin);

#endif /* HR_GPIO_H */
