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
	void cloneThread(kiv_hal::TRegisters& registers);
	void processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock);
	void threadStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock);

	void invalidWaitForRequest(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle);


	const std::unique_ptr<ProcessControlBlock> pcb = std::make_unique<ProcessControlBlock>();


};