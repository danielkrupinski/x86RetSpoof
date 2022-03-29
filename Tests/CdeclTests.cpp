#include <array>
#include <cstdint>
#include <functional>
#include <intrin.h>
#include <limits>

#include <gtest/gtest.h>

#include "Gadget.h"
#include "x86RetSpoof.h"

static void* __cdecl getReturnAddressOfMyself()
{
    return _ReturnAddress();
}

TEST(InvokeCdeclTest, ReturnAddressOfTheInvokedFunctionIsTheAddressOfTheGadget) {
    EXPECT_EQ(x86RetSpoof::invokeCdecl<void*>(std::uintptr_t(&getReturnAddressOfMyself), std::uintptr_t(gadget.data())), gadget.data());
}

TEST(InvokeCdeclTest, 64bitIntegerIsReturnedCorrectly) {
    static constexpr std::uint64_t value = (std::numeric_limits<std::uint64_t>::max)();
    std::uint64_t(__cdecl* const function)() = []{ return value; };
    EXPECT_EQ(x86RetSpoof::invokeCdecl<std::uint64_t>(std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeCdeclTest, FloatIsReturnedCorrectly) {
    static constexpr float value = 3.1415f;
    float(__cdecl* const function)() = []{ return value; };
    EXPECT_FLOAT_EQ(x86RetSpoof::invokeCdecl<float>(std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeCdeclTest, ExplicitReferenceArgumentIsNotCopied) {
    void(__cdecl* const function)(unsigned& value) = [](unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeCdecl<void, unsigned&>(std::uintptr_t(function), std::uintptr_t(gadget.data()), number);
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeCdeclTest, ReferenceArgumentIsDeducedCorrectly) {
    void(__cdecl* const function)(unsigned& value) = [](unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeCdecl<void>(std::uintptr_t(function), std::uintptr_t(gadget.data()), std::ref(number));
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeCdeclTest, LvalueArgumentIsNotDeducedToReference) {
    std::uintptr_t(__cdecl* const function)(std::uintptr_t) = [](std::uintptr_t value) { return value; };
    std::uintptr_t number = 1234;
    EXPECT_EQ(x86RetSpoof::invokeCdecl<std::uintptr_t>(std::uintptr_t(function), std::uintptr_t(gadget.data()), number), std::uintptr_t(1234));
}
