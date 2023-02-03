#include <array>
#include <cstdint>
#include <functional>
#include <intrin.h>
#include <limits>

#include <gmock/gmock.h>
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

TEST(InvokeCdeclTest, FunctionIsInvokedOncePerCall) {
    struct Mock {
        MOCK_METHOD(void, called, (), (const));
    };
    static std::unique_ptr<Mock> mock;
    mock = std::make_unique<Mock>();

    void(__cdecl* const function)() = []() { mock->called(); };
    EXPECT_CALL(*mock.get(), called());
    x86RetSpoof::invokeCdecl<void>(std::uintptr_t(function), std::uintptr_t(gadget.data()));
    mock.reset();
}

#define TEST_REGISTER_PRESERVED(registerName) \
TEST(InvokeCdeclTest, registerName##RegisterIsPreserved) { \
    void(__cdecl* const invokeCdecl)(std::uintptr_t, x86RetSpoof::detail::Context*, std::uintptr_t) = x86RetSpoof::detail::invokeCdecl<void>; \
    void(__cdecl* const function)() = [] {}; \
    x86RetSpoof::detail::Context context; \
    const auto addressOfGadget = std::uintptr_t(gadget.data()); \
\
    std::uintptr_t registerName##BeforeCall = 0; \
    std::uintptr_t registerName##AfterCall = 0; \
    __asm { \
        __asm mov registerName##BeforeCall, registerName \
        __asm push addressOfGadget \
        __asm lea eax, context \
        __asm push eax \
        __asm push function \
        __asm call invokeCdecl \
        __asm add esp, 12 \
        __asm mov registerName##AfterCall, registerName \
    } \
\
    EXPECT_EQ(registerName##BeforeCall, registerName##AfterCall); \
}

TEST_REGISTER_PRESERVED(ebx);
TEST_REGISTER_PRESERVED(ebp);
TEST_REGISTER_PRESERVED(esi);
TEST_REGISTER_PRESERVED(edi);
