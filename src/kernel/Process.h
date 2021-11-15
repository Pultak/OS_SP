#pragma once
#include "../../src/api/api.h"

#include <memory.h>
#include <memory>
#include <iostream>
#include <map>
#include <filesystem>
#include "ProcessControlBlock.h"
#include "ThreadControlBlock.h"
#include "Windows.h"
#include "SleepListener.h"
#include "Synchronization.h"

enum class ProcessState {
	Ready = 0,
	Running = 1,
	Waiting = 2,
	Terminated = 3
};

size_t __stdcall defaultSignalHandler(const kiv_hal::TRegisters& regs) {
	auto signal_id = static_cast<kiv_os::NSignal_Id>(regs.rcx.l);
	
	switch (signal_id) {
	case kiv_os::NSignal_Id::Terminate:
		//todo kill the system or something idk
		break;
	}


	return 0;
}


class Process {
public:
	static void HandleProcess(kiv_hal::TRegisters& registers, HMODULE user_programs);

private:
	static void clone(kiv_hal::TRegisters& registers, HMODULE userSpaceLib);

	static void waitFor(kiv_hal::TRegisters& registers);

	static void exit(kiv_hal::TRegisters& registers);

	static void readExitCode(kiv_hal::TRegisters& registers);

	static void shutdown();

	static void registerSignalHandler(kiv_hal::TRegisters& registers);

private:
	static void cloneProcess(kiv_hal::TRegisters& registers, HMODULE userSpaceLib);

	static void processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc t_threadproc);
	static void threadStartPoint();

	static void invalidWaitForRequest(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle);


public:
	static const std::unique_ptr<ProcessControlBlock> pcb;


public:
	/// <summary>
	/// External parameters of process
	/// </summary>
	kiv_os::THandle handle;
	kiv_os::THandle stdInput;
	kiv_os::THandle stdOutput;
	ProcessState state = ProcessState::Ready;
	kiv_os::NOS_Error exitCode = kiv_os::NOS_Error::Success;
	char* programName;

	std::filesystem::path workingDirectory;

	std::map<kiv_os::NSignal_Id, kiv_os::TThread_Proc> signalHandlers;

private:
	/// <summary>
	/// Internal parameters of process
	/// </summary>
	std::shared_ptr<ThreadControlBlock> tcb;
	std::list<SleepListener*> listeners;
	
	Synchronization::Spinlock* listenersLock = new Synchronization::Spinlock(0);
/*
private:
	
	void addListener(SleepListener* listener);
	*/

public:

	Process(kiv_os::THandle handle, kiv_os::THandle stdIn, kiv_os::THandle stdOut, char* program): handle(handle), stdInput(stdIn), stdOutput(stdOut), programName(program) {
		
		//no other signals are pressent atm
		signalHandlers[kiv_os::NSignal_Id::Terminate] = defaultSignalHandler;
	}

	~Process() {
		//todo check exit codes by parent -> block process?
		delete listenersLock;
	}

};
