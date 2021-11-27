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

	/// <summary>
	/// Starting function of the newly created process
	/// </summary>
	/// <param name="registers">Process registers</param>
	/// <param name="userProgram">executable program from userspace</param>
	/// <param name="lock">synchronization lock to unlock when the initialization is done</param>
	void processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock);
	
	/// <summary>
	/// Starting function of the newly created thread
	/// </summary>
	/// <param name="registers">Thread registers</param>
	/// <param name="userProgram">executable program from userspace</param>
	/// <param name="lock">synchronization lock to unlock when the initialization is done</param>
	void threadStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock);

	/// <summary>
	/// Function used for WaitFor
	/// Removes assigned listener from all passed handles
	/// </summary>
	/// <param name="alreadyDone">count of already passed handles</param>
	/// <param name="handles">array of passed handles</param>
	/// <param name="thisHandle">handle of the waiting process</param>
	void removeAssignedListener(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle);

	/// <summary>
	/// Process control block
	/// </summary>
	const std::unique_ptr<ProcessControlBlock> pcb = std::make_unique<ProcessControlBlock>();


};