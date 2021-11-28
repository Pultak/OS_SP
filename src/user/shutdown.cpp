#pragma once

#include "shutdown.h"

size_t __stdcall shutdown(const kiv_hal::TRegisters& regs)
{
	kiv_os_rtl::Shutdown();
	return 0;
}
