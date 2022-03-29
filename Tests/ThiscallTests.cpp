#include <array>
#include <cstdint>
#include <functional>
#include <intrin.h>
#include <limits>

#include <gtest/gtest.h>

#include "Gadget.h"
#include "x86RetSpoof.h"

static void* __fastcall getReturnAddressOfMyself()
{
    return _ReturnAddress();
}

TEST(InvokeThiscallTest, ReturnAddressOfTheInvokedFunctionIsTheAddressOfTheGadget) {
    EXPECT_EQ(x86RetSpoof::invokeThiscall<void*>(0, std::uintptr_t(&getReturnAddressOfMyself), std::uintptr_t(gadget.data())), gadget.data());
}

TEST(InvokeThiscallTest, 64bitIntegerIsReturnedCorrectly) {
    static constexpr std::uint64_t value = (std::numeric_limits<std::uint64_t>::max)();
    std::uint64_t(__fastcall* const function)() = []{ return value; };
    EXPECT_EQ(x86RetSpoof::invokeThiscall<std::uint64_t>(0, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeThiscallTest, FloatIsReturnedCorrectly) {
    static constexpr float value = 3.1415f;
    float(__fastcall* const function)() = []{ return value; };
    EXPECT_FLOAT_EQ(x86RetSpoof::invokeThiscall<float>(0, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeThiscallTest, EcxContainsCorrectValueInTheInvokedFunction) {
    static constexpr int value = 12345;
    int(__fastcall* const function)(int ecx) = [](int ecx) { return ecx; };
    EXPECT_EQ(x86RetSpoof::invokeThiscall<int>(value, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeThiscallTest, ExplicitReferenceArgumentIsNotCopied) {
    void(__fastcall* const function)(int ecx, int edx, unsigned& value) = [](int, int, unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeThiscall<void, unsigned&>(0, std::uintptr_t(function), std::uintptr_t(gadget.data()), number);
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeThiscallTest, ReferenceArgumentIsDeducedCorrectly) {
    void(__fastcall* const function)(int ecx, int edx, unsigned& value) = [](int, int, unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeThiscall<void>(0, std::uintptr_t(function), std::uintptr_t(gadget.data()), std::ref(number));
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeThiscallTest, LvalueArgumentIsNotDeducedToReference) {
    std::uintptr_t(__fastcall* const function)(int ecx, int edx, std::uintptr_t) = [](int, int, std::uintptr_t value) { return value; };
    std::uintptr_t number = 1234;
    EXPECT_EQ(x86RetSpoof::invokeThiscall<std::uintptr_t>(0, std::uintptr_t(function), std::uintptr_t(gadget.data()), number), std::uintptr_t(1234));
}
