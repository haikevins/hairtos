#ifndef HR_HOOKS_H
#define HR_HOOKS_H

#include <stdint.h>

#include "hr_diagnostics.h"
#include "hr_types.h"

void hr_hook_panic(const hr_panic_record_t *record);
void hr_hook_stack_overflow(const hr_task_t *task, const char *task_name);
void hr_hook_assert_failed(const char *expression,
                           const char *source_file,
                           uint32_t source_line);

_Noreturn void hr_assert_failed(const char *expression,
                      const char *source_file,
                      uint32_t source_line);

#if (HR_CFG_ENABLE_ASSERT == 1)
#define HR_ASSERT(expression)                                                     \
    do                                                                            \
    {                                                                             \
        if (!(expression))                                                        \
        {                                                                         \
            hr_assert_failed(#expression, __FILE__, (uint32_t)__LINE__);          \
        }                                                                         \
    } while (0)
#else
#define HR_ASSERT(expression) ((void)sizeof(expression))
#endif

#endif /* HR_HOOKS_H */
