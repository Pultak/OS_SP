#pragma once
#include "../api/hal.h"
#include "Process.h"
#include "handles.h"
#include "ProcessControlBlock.h"
namespace ProcessUtils {
	size_t __stdcall defaultSignalHandler(const kiv_hal::TRegisters& regs);

	void HandleProcess(kiv_hal::TRegisters& registers, HMODULE user_programs);

	void clone(kiv_hal::TRegisters& registers, HMODULE userSpaceLib);

	void waitFor(kiv_hal::TRegisters& registers);

	void exit(kiv_hal::TRegisters& registers);

	void readExitCode(kiv_hal::TRegisters& registers);

	void shutdown();

	void registerSignalHandler(kiv_hal::TRegisters& registers);

	void cloneProcess(kiv_hal::TRegisters& registers, HMODULE userSpaceLib);
	void processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc t_threadproc);
	void threadStartPoint();

	void invalidWaitForRequest(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle);


	const std::unique_ptr<ProcessControlBlock> pcb;


};