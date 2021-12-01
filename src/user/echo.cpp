#include "echo.h"
#include <iostream>

size_t __stdcall echo(const kiv_hal::TRegisters& regs) {
	const kiv_os::THandle std_out = static_cast<kiv_os::THandle>(regs.rbx.x);
	size_t counter = 0;
	const char* new_line = "\n";
	const char* print = reinterpret_cast<const char*>(regs.rdi.r);
	char etx[1];
	etx[0] = 3;
	kiv_os_rtl::Write_File(std_out, print, strlen(print), counter);
	kiv_os_rtl::Write_File(std_out, new_line, 1, counter);
	kiv_os_rtl::Write_File(std_out, etx, 1, counter);
	return 0;
}