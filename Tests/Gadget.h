#include <array>
#include <cstdint>

#pragma section(".text")
__declspec(allocate(".text")) inline constexpr std::array<std::uint8_t, 2> gadget{ 0xFF, 0x23 }; // jmp dword ptr[ebx]
