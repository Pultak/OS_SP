#pragma once

#include "kernel.h"
#include "io.h"
//#include "filesystems.h"
#include <Windows.h>

HMODULE User_Programs;


void Initialize_Kernel() {
	User_Programs = LoadLibraryW(L"user.dll");
	filesystems::InitFilesystems();
}

void Shutdown_Kernel() {
	FreeLibrary(User_Programs);
	//todo clear processes
	//delete ProcessUtils::pcb;
}

void __stdcall Sys_Call(kiv_hal::TRegisters &regs) {
	regs.flags.carry = 0;

	switch (static_cast<kiv_os::NOS_Service_Major>(regs.rax.h)) {
		
		case kiv_os::NOS_Service_Major::File_System:		
			io::Handle_IO(regs);
			break;
		case kiv_os::NOS_Service_Major::Process:
			ProcessUtils::HandleProcess(regs, User_Programs);
			break;
	}

}

void __stdcall Bootstrap_Loader(kiv_hal::TRegisters &context) {
	Initialize_Kernel();
	kiv_hal::Set_Interrupt_Handler(kiv_os::System_Int_Number, Sys_Call);

	kiv_hal::TRegisters regs{};
	
	//get address of shell function
	kiv_os::TThread_Proc shell = (kiv_os::TThread_Proc)GetProcAddress(User_Programs, "shell");
	if (shell) {
		//first open STDIN and STDOUT
		regs.rdx.r = reinterpret_cast<uint64_t>("\\stdout\\");
		io::OpenIOHandle(regs);
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}
		kiv_os::THandle std_out = regs.rax.x;

		regs.rdx.r = reinterpret_cast<uint64_t>("\\stdin\\");
		io::OpenIOHandle(regs);
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}
		kiv_os::THandle std_in = regs.rax.x;

		char* shellFunction = "shell\0";
		//rax for HandleProcess switch -> not needed now 
		regs.rbx.e = (std_in << 16) | std_out; //specify standard input output
		regs.rcx.l = static_cast<uint8_t>(kiv_os::NClone::Create_Process); //CreateProcess switch
		regs.rdx.r = reinterpret_cast<uint64_t>(shellFunction); //pass the function address to the new process
		regs.rdi.r = reinterpret_cast<uint64_t>(""); //no arguments atm

		ProcessUtils::clone(regs, User_Programs);	
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}
		
		auto processHandle = static_cast<kiv_os::THandle>(regs.rax.x); //get the handle of the newly created process
		kiv_os::THandle handles[1];
		handles[0] = processHandle;
		regs.rcx.l = 1; // Only one handle
		regs.rdx.r = reinterpret_cast<uint64_t>(handles);
		//waiting for the child process
		ProcessUtils::waitFor(regs);

		regs.rdx.x = processHandle;
		ProcessUtils::readExitCode(regs);
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}

		//close STDIN and STDOUT
		regs.rdx.x = std_in;
		io::CloseIOHandle(regs);
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}

		regs.rdx.x = std_out;
		io::CloseIOHandle(regs);
		if (regs.flags.carry) {
			//something failed -> copy error code
			context.flags.carry = 1;
			context.rax.r = regs.rax.x;
			Shutdown_Kernel();
			return;
		}

	}
	else {
		//shell function not found
		Set_Error(true, context);
	}
	Shutdown_Kernel();
}


void Set_Error(const bool failed, kiv_hal::TRegisters &regs) {
	if (failed) {
		regs.flags.carry = true;
		regs.rax.r = GetLastError();
	}
	else
		regs.flags.carry = false;
}