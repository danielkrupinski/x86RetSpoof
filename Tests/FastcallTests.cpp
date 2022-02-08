#include <array>
#include <cstdint>
#include <intrin.h>
#include <limits>

#include <gtest/gtest.h>

#include "x86RetSpoof.h"

#pragma section(".text")
__declspec(allocate(".text")) constexpr std::array<std::uint8_t, 2> gadget{ 0xFF, 0x23 }; // jmp dword ptr[ebx]

static void* __fastcall getReturnAddressOfMyself()
{
    return _ReturnAddress();
}

TEST(InvokeFastcallTest, ReturnAddressOfTheInvokedFunctionIsTheAddressOfTheGadget) {
    EXPECT_EQ(x86RetSpoof::invokeFastcall<void*>(0, 0, std::uintptr_t(&getReturnAddressOfMyself), std::uintptr_t(gadget.data())), gadget.data());
}

TEST(InvokeFastcallTest, 64bitIntegerIsReturnedCorrectly) {
    static constexpr std::uint64_t value = (std::numeric_limits<std::uint64_t>::max)();
    std::uint64_t(__fastcall* const function)() = []{ return value; };
    EXPECT_EQ(x86RetSpoof::invokeFastcall<std::uint64_t>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}
