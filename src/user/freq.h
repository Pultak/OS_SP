#pragma once

#include "rtl.h"

//void freq_prot(const kiv_hal::TRegisters& regs);
extern "C" size_t __stdcall freq(const kiv_hal::TRegisters& regs);
