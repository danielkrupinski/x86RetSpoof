#include <array>
#include <cstdint>
#include <intrin.h>
#include <limits>

#include <gtest/gtest.h>

#include "Gadget.h"
#include "x86RetSpoof.h"

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

TEST(InvokeFastcallTest, FloatIsReturnedCorrectly) {
    static constexpr float value = 3.1415f;
    float(__fastcall* const function)() = []{ return value; };
    EXPECT_FLOAT_EQ(x86RetSpoof::invokeFastcall<float>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeFastcallTest, EcxContainsCorrectValueInTheInvokedFunction) {
    static constexpr int value = 12345;
    int(__fastcall* const function)(int ecx) = [](int ecx) { return ecx; };
    EXPECT_EQ(x86RetSpoof::invokeFastcall<int>(value, 0, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeFastcallTest, EdxContainsCorrectValueInTheInvokedFunction) {
    static constexpr int value = 12345;
    int(__fastcall* const function)(int ecx, int edx) = []([[maybe_unused]] int ecx, int edx) { return edx; };
    EXPECT_EQ(x86RetSpoof::invokeFastcall<int>(0, value, std::uintptr_t(function), std::uintptr_t(gadget.data())), value);
}

TEST(InvokeFastcallTest, EcxAndEdxContainCorrectValueInTheInvokedFunction) {
    unsigned(__fastcall* const function)(unsigned, unsigned) = [](unsigned ecx, unsigned edx) { return (ecx << 16) | edx; };
    EXPECT_EQ(x86RetSpoof::invokeFastcall<unsigned>(0xDEAD, 0xBEEF, std::uintptr_t(function), std::uintptr_t(gadget.data())), 0xDEADBEEF);
}

TEST(InvokeFastcallTest, ExplicitReferenceArgumentIsPassedCorrectly) {
    void(__fastcall* const function)(int ecx, int edx, unsigned& value) = [](int, int, unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeFastcall<void, unsigned&>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data()), number);
    EXPECT_EQ(number, 0xDEADBEEF);
}
