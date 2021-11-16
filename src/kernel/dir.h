#pragma once
#include <filesystem>
#include "filesystems.h"
#include "../api/api.h"

bool Set_Working_Dir(kiv_hal::TRegisters& regs);

bool Get_Working_Dir(kiv_hal::TRegisters& regs);

bool Get_Working_Dir(char* out_buffer, size_t buffer_size, size_t& written);
