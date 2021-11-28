#include "echo.h"
#include <iostream>

size_t __stdcall echo(const kiv_hal::TRegisters& regs) {
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	size_t counter = 0;
	const char* new_line = "\n";
	const char* print = reinterpret_cast<const char*>(regs.rdi.r);

	kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
	kiv_os_rtl::Write_File(std_out, new_line, strlen(new_line), counter);
	return 0;
}