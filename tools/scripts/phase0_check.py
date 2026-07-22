#!/usr/bin/env python3
from pathlib import Path
import sys
root=Path(__file__).resolve().parents[2]
required=['README.md','config/hairtos_config.h','config/hairevent_config.h',
'docs/architecture.md','docs/dependency-rules.md','docs/task-model.md',
'docs/scheduler.md','docs/context-switch.md','docs/interrupt-model.md',
'docs/timeout-model.md','docs/event-framework.md','docs/memory-model.md',
'docs/testing-guide.md','docs/roadmap.md','docs/requirements-coverage.md','kernel/include/hairtos/hairtos.h',
'framework/event/include/hairevent/he_event.h','arch/arm/cortex-m3/include/hr_port.h',
'soc/stm32f1/startup_stm32f103.S','boards/bluepill_f103c8/STM32F103C8Tx_FLASH.ld']
missing=[x for x in required if not (root/x).exists()]
config=(root/'config/hairtos_config.h').read_text()
decisions=['HR_CFG_TICK_RATE_HZ','HR_CFG_PRIORITY_COUNT','HR_CFG_PREEMPTION',
'HR_CFG_TIME_SLICING','HR_CFG_STATIC_ALLOCATION','HR_CFG_USE_FPU',
'HR_CFG_USE_MPU','HR_CFG_SINGLE_CORE']
missing_decisions=[x for x in decisions if x not in config]
if missing or missing_decisions:
 print('Missing:',missing,missing_decisions); sys.exit(1)
print('HairRTOS Phase 0 check: PASS')
print('Required files:',len(required))
print('Configuration decisions:',len(decisions))
