#pragma once


#include <Windows.h>


#include "..\api\hal.h"
#include "..\api\api.h"
#include "io.h"
#include "Process.h"

void Set_Error(const bool failed, kiv_hal::TRegisters &regs);
void __stdcall Bootstrap_Loader(kiv_hal::TRegisters &context);
