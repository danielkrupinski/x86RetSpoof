#include <array>
#include <cstdint>
#include <intrin.h>

#include <gtest/gtest.h>

#include "x86RetSpoof.h"

#pragma section(".text")
__declspec(allocate(".text")) constexpr std::array<std::uint8_t, 2> gadget{ 0xFF, 0x23 }; // jmp dword ptr[ebx]

static void* __stdcall getReturnAddressOfMyself()
{
    return _ReturnAddress();
}

TEST(InvokeStdcallTest, ReturnAddressOfTheInvokedFunctionIsTheAddressOfTheGadget) {
    EXPECT_EQ(x86RetSpoof::invokeStdcall<void*>(std::uintptr_t(&getReturnAddressOfMyself), std::uintptr_t(gadget.data())), gadget.data());
}

TEST(InvokeStdcallTest, 64bitIntegerIsReturnedCorrectly) {
    static constexpr std::uint64_t value = 0xFEDCBA98764321;
    std::uint64_t(__stdcall* const function)() = []{ return value; };
    EXPECT_EQ(x86RetSpoof::invokeStdcall<std::uint64_t>(std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}
