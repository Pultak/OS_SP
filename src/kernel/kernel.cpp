#pragma once

#include "kernel.h"
#include "io.h"
#include "filesystems.h"
#include <Windows.h>

HMODULE User_Programs;


void Initialize_Kernel() {
	User_Programs = LoadLibraryW(L"user.dll");
	InitFilesystems();
}

void Shutdown_Kernel() {
	FreeLibrary(User_Programs);
}

void __stdcall Sys_Call(kiv_hal::TRegisters &regs) {

	switch (static_cast<kiv_os::NOS_Service_Major>(regs.rax.h)) {
		
		case kiv_os::NOS_Service_Major::File_System:		
			Handle_IO(regs);
			break;
		case kiv_os::NOS_Service_Major::Process:
			ProcessUtils::HandleProcess(regs, User_Programs);
			break;
	}

}

void __stdcall Bootstrap_Loader(kiv_hal::TRegisters &context) {
	Initialize_Kernel();
	kiv_hal::Set_Interrupt_Handler(kiv_os::System_Int_Number, Sys_Call);

	//v ramci ukazky jeste vypiseme dostupne disky
	kiv_hal::TRegisters regs{};
	
	
	//spustime shell - v realnem OS bychom ovsem spousteli login
	kiv_os::TThread_Proc shell = (kiv_os::TThread_Proc)GetProcAddress(User_Programs, "shell");
	if (shell) {
		//spravne se ma shell spustit pres clone!
		//ale ten v kostre pochopitelne neni implementovan		
		shell(regs);
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