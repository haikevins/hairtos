# VS Code IntelliSense

Use the **hairtos - All Sources** configuration when browsing the whole repository.
It intentionally exposes internal include directories to the editor only, so files
outside the currently selected CMake example do not show false include errors.
The actual compiler boundary remains enforced by CMake target include directories.

For exact per-file compile definitions, run:

```bash
make EXAMPLE=16-diagnostics-stress-stabilization ENVIRONMENT=target TOOLCHAIN=clang intellisense
```

Then select **hairtos - Active CMake Build**.

After replacing the configuration, run these commands from the VS Code command palette:

1. `C/C++: Select a Configuration`
2. Select `hairtos - All Sources`
3. `C/C++: Reset IntelliSense Database`
4. `Developer: Reload Window`

Open the repository root containing `Makefile`; do not open one example directory by itself.
Disable the clangd extension for this workspace when using Microsoft C/C++ IntelliSense.
