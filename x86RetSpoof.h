/*
x86RetSpoof - https://github.com/danielkrupinski/x86RetSpoof

Invoke functions with a spoofed return address.
For 32-bit Windows binaries.
Supports __fastcall, __thiscall, __stdcall and __cdecl calling conventions.
Written in C++17.

Version: 17 February 2023
*/

/*
MIT License

Copyright (c) 2022-2023 Daniel Krupiński

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cstdint>
#include <utility>

namespace x86RetSpoof
{
    namespace detail
    {
        struct Context;
    }

    template <typename ReturnType, typename... StackArgs>
    ReturnType invokeFastcall(std::uintptr_t ecx, std::uintptr_t edx, std::uintptr_t functionAddress, std::uintptr_t gadgetAddress, StackArgs... stackArgs) noexcept
    {
        detail::Context context;
        return invokeFastcall<ReturnType, StackArgs...>(ecx, edx, functionAddress, &context, gadgetAddress, stackArgs...);
    }

    template <typename ReturnType, typename... StackArgs>
    ReturnType invokeThiscall(std::uintptr_t ecx, std::uintptr_t functionAddress, std::uintptr_t gadgetAddress, StackArgs... stackArgs) noexcept
    {
        return invokeFastcall<ReturnType, StackArgs...>(ecx, 0, functionAddress, gadgetAddress, stackArgs...);
    }

    template <typename ReturnType, typename... StackArgs>
    ReturnType invokeStdcall(std::uintptr_t functionAddress, std::uintptr_t gadgetAddress, StackArgs... stackArgs) noexcept
    {
        return invokeThiscall<ReturnType, StackArgs...>(0, functionAddress, gadgetAddress, stackArgs...);
    }

    template <typename ReturnType, typename... StackArgs>
    ReturnType invokeCdecl(std::uintptr_t functionAddress, std::uintptr_t gadgetAddress, StackArgs... stackArgs) noexcept
    {
        detail::Context context;
        return invokeCdecl<ReturnType, StackArgs...>(functionAddress, &context, gadgetAddress, stackArgs...);
    }

    namespace detail
    {
        struct Context {
            std::uintptr_t ebxBackup;
            std::uintptr_t addressToJumpToInGadget;
            std::uintptr_t invokerReturnAddress;
        };

        template <typename ReturnType, typename... StackArgs>
        __declspec(naked) ReturnType __fastcall invokeFastcall(std::uintptr_t /*ecx*/, std::uintptr_t /*edx*/, std::uintptr_t /*functionAddress*/, Context* /*context*/, std::uintptr_t /*gadgetAddress*/, StackArgs... /*stackArgs*/) noexcept
        {
            __asm {
                mov eax, [esp + 8] // load a pointer to context into eax
                mov [eax], ebx // save ebx in context.ebxBackup
                lea ebx, returnHereFromGadget // load the address of the label we want the gadget to jump to
                mov [eax + 4], ebx // save the address of 'returnHereFromGadget' in context.addressToJumpToInGadget
                pop dword ptr [eax + 8] // pop return address from stack into context.invokerReturnAddress

                lea ebx, [eax + 4] // load the address of context.addressToJumpToInGadget to ebx
                ret 4 // pop 'functionAddress' from stack and jump to it, skip 'context' on stack; esp will point to the spoofed return address (gadgetAddress)

             returnHereFromGadget:
                push [ebx + 4] // restore context.invokerReturnAddress as a return address
                mov ebx, [ebx - 4] // restore ebx from context.ebxBackup 
                ret
            }
        }

        template <typename ReturnType, typename... StackArgs>
        __declspec(naked) ReturnType __cdecl invokeCdecl(std::uintptr_t /*functionAddress*/, Context* /*context*/, std::uintptr_t /*gadgetAddress*/, StackArgs... /*stackArgs*/) noexcept
        {
            __asm {
                mov eax, [esp + 8] // load a pointer to context into eax
                mov [eax], ebx // save ebx in context.ebxBackup
                lea ebx, returnHereFromGadget // load the address of the label we want the gadget to jump to
                mov [eax + 4], ebx // save the address of 'returnHereFromGadget' in context.addressToJumpToInGadget
                pop dword ptr [eax + 8] // pop return address from stack into context.invokerReturnAddress

                lea ebx, [eax + 4] // load the address of context.addressToJumpToInGadget to ebx
                ret 4 // pop 'functionAddress' from stack and jump to it, skip 'context' on stack; esp will point to the spoofed return address (gadgetAddress)

             returnHereFromGadget:
                sub esp, 12
                push [ebx + 4] // restore context.invokerReturnAddress as a return address
                mov ebx, [ebx - 4] // restore ebx from context.ebxBackup 
                ret
            }
        }
    }
}
