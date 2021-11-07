#pragma once
#include "../../src/api/api.h"

#include <memory.h>
#include <iostream>
#include <map>
#include "ProcessControlBlock.h"
#include "ThreadControlBlock.h"
#include "Windows.h"

enum class Process_Status {
	Ready = 0,
	Running = 1,
	Zombie = 2
};

class Process {
public:
	static void Handle_Process(kiv_hal::TRegisters& registers, HMODULE user_programs);

private:
	static void clone(kiv_hal::TRegisters& registers, HMODULE user_programs);

	static void wait_for(kiv_hal::TRegisters& registers);

	static void exit(kiv_hal::TRegisters& registers);

	static void exit(kiv_os::THandle handle, kiv_os::NOS_Error exit_code);

	static void read_exit_code(kiv_hal::TRegisters& registers);

	static void register_signal_handler(kiv_hal::TRegisters& registers);

	static void signal_all_processes(kiv_os::NSignal_Id signal);

	static void signal(kiv_os::NSignal_Id signal_id, Process* process);

	static void shutdown();

public:
	static std::shared_ptr<ProcessControlBlock*> pcb;

public:
	std::shared_ptr <ThreadControlBlock*> tcb;

public:

	kiv_os::THandle handle;
	kiv_os::THandle std_in;
	kiv_os::THandle std_out;
	Process_Status status = Process_Status::Ready;
	kiv_os::NOS_Error exit_code;
	char program_name[42];
	//todo change to filesystem path
	char working_directory[256];



	Process(kiv_hal::TRegisters regs, kiv_os::THandle handle, kiv_os::THandle std_in, kiv_os::THandle std_out, char* program) {
		registers = std::make_unique<kiv_hal::TRegisters>(regs);
		//todo other parameters
	}

	~Process() {
		//todo check exit codes by parent -> block process?
	}

private:
	std::unique_ptr<kiv_hal::TRegisters> registers;
	static std::map<kiv_os::THandle, std::unique_ptr<Process>> pcb;
};
