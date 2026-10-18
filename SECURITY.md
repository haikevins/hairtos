# Security Policy

## Project scope

`hairtos` is an educational and experimental RTOS. It is not safety-certified, security-certified, or production-qualified, and no security response SLA is guaranteed.

Security reports are still welcome, especially for issues involving memory safety, privilege or exception handling, malformed input reaching kernel/framework code, unsafe driver behavior, or documentation that could cause users to make incorrect security assumptions.

## Supported code

Security fixes are considered on a best-effort basis for the current `main` branch and the latest published release candidate or release. Older snapshots and historical commits are not maintained as supported security branches.

## Reporting a vulnerability

Please do **not** disclose exploit details, proof-of-concept code, secrets, or sensitive device information in a public GitHub issue.

Preferred reporting path:

1. Use GitHub's private vulnerability reporting / security advisory flow for this repository when it is available under the **Security** tab.
2. If private reporting is not available, open a minimal public issue that contains no vulnerability details and requests a private contact channel from the maintainer.

Include, when possible:

- affected commit or release;
- affected target and configuration;
- whether the issue is host-only, target-specific, or reproducible on physical hardware;
- a concise impact description;
- reproduction conditions without publishing sensitive material;
- any relevant sanitizer, fault, panic, debugger, or register evidence.

## Security-related validation

Host AddressSanitizer/UndefinedBehaviorSanitizer results cover only the exercised native code paths. They do not establish Cortex-M exception safety, interrupt timing, memory-protection properties, electrical safety, secure boot, isolation, or resistance to malicious physical access.

Any security claim tied to the STM32F103 target should be supported by target-specific evidence rather than inferred from host CI.
