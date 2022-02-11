# x86RetSpoof [![Windows](https://github.com/danielkrupinski/x86RetSpoof/actions/workflows/windows.yml/badge.svg?event=push)](https://github.com/danielkrupinski/x86RetSpoof/actions/workflows/windows.yml)
Invoke functions with a spoofed return address. For 32-bit Windows binaries.

# How to use
1. Include x86RetSpoof.h in your project.
2. Find `FF 23` byte sequence (`gadget`, machine code equivalent of `jmp dword ptr [ebx]`) in the executable code section of the module you want the spoofed return address to appear in. The address of it will be the `gadgetAddress` and the invoked function will see it as the return address.
3. Call the function with `x86RetSpoof::invoke...()` matching the calling convention of the target function.

## Example
Calling [MessageBoxW](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-messageboxw) function:
```cpp
x86RetSpoof::invokeStdcall<int>(std::uintptr_t(&MessageBoxW), std::uintptr_t(gadgetAddress), nullptr, L"text", L"title", MB_OK);
```
