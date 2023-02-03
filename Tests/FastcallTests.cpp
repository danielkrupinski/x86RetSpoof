#include <array>
#include <cstdint>
#include <functional>
#include <intrin.h>
#include <limits>

#include <gmock/gmock.h>
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

TEST(InvokeFastcallTest, ExplicitReferenceArgumentIsNotCopied) {
    void(__fastcall* const function)(int ecx, int edx, unsigned& value) = [](int, int, unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeFastcall<void, unsigned&>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data()), number);
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeFastcallTest, ReferenceArgumentIsDeducedCorrectly) {
    void(__fastcall* const function)(int ecx, int edx, unsigned& value) = [](int, int, unsigned& value) { value = 0xDEADBEEF; };
    unsigned number = 0;
    x86RetSpoof::invokeFastcall<void>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data()), std::ref(number));
    EXPECT_EQ(number, 0xDEADBEEF);
}

TEST(InvokeFastcallTest, LvalueArgumentIsNotDeducedToReference) {
    std::uintptr_t(__fastcall* const function)(int ecx, int edx, std::uintptr_t) = [](int, int, std::uintptr_t value) { return value; };
    std::uintptr_t number = 1234;
    EXPECT_EQ(x86RetSpoof::invokeFastcall<std::uintptr_t>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data()), number), std::uintptr_t(1234));
}

TEST(InvokeFastcallTest, FunctionIsInvokedOncePerCall) {
    struct Mock {
        MOCK_METHOD(void, called, (), (const));
    };
    static std::unique_ptr<Mock> mock;
    mock = std::make_unique<Mock>();

    void(__fastcall* const function)() = []{ mock->called(); };
    EXPECT_CALL(*mock.get(), called());
    x86RetSpoof::invokeFastcall<void>(0, 0, std::uintptr_t(function), std::uintptr_t(gadget.data()));
    mock.reset();
}

#define TEST_REGISTER_PRESERVED(registerName) \
TEST(InvokeFastcallTest, registerName##RegisterIsPreserved) { \
    void(__fastcall* const invokeFastcall)(std::uintptr_t, std::uintptr_t, std::uintptr_t, x86RetSpoof::detail::Context*, std::uintptr_t) = x86RetSpoof::detail::invokeFastcall<void>; \
    void(__fastcall* const function)() = [] {}; \
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
        __asm xor ecx, ecx \
        __asm xor edx, edx \
        __asm call invokeFastcall \
        __asm mov registerName##AfterCall, registerName \
    } \
\
    EXPECT_EQ(registerName##BeforeCall, registerName##AfterCall); \
}

TEST_REGISTER_PRESERVED(ebx);
TEST_REGISTER_PRESERVED(ebp);
TEST_REGISTER_PRESERVED(esi);
TEST_REGISTER_PRESERVED(edi);
