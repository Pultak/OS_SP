#include "io.h"
#include "kernel.h"
#include "dir.h"
#include "filesystems.h"
#include "handles.h"
#include "PipeUtils.h"

size_t Read_Line_From_Console(char *buffer, const size_t buffer_size) {
	kiv_hal::TRegisters registers;
	
	size_t pos = 0;
	while (pos < buffer_size) {
		//read char
		registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NKeyboard::Read_Char);
		kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Keyboard, registers);
		
		if (!registers.flags.non_zero) break;	//nic jsme neprecetli, 
												//pokud je rax.l EOT, pak byl zrejme vstup korektne ukoncen
												//jinak zrejme doslo k chybe zarizeni

		char ch = registers.rax.l;

		//osetrime zname kody
		switch (static_cast<kiv_hal::NControl_Codes>(ch)) {
			case kiv_hal::NControl_Codes::BS: {
					//mazeme znak z bufferu
					if (pos > 0) pos--;

					registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_Control_Char);
					registers.rdx.l = ch;
					kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
				}
				break;

			case kiv_hal::NControl_Codes::LF:  break;	//jenom pohltime, ale necteme
			case kiv_hal::NControl_Codes::NUL:			//chyba cteni?
			case kiv_hal::NControl_Codes::EOT:			//konec textu
			case kiv_hal::NControl_Codes::CR:  return pos;	//docetli jsme az po Enter


			default: buffer[pos] = ch;
					 pos++;	
					 registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_String);
					 registers.rdx.r = reinterpret_cast<decltype(registers.rdx.r)>(&ch);
					 registers.rcx.r = 1;
					 kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
					 break;
		}
	}

	return pos;

}

kiv_os::THandle io::addIoHandle(IOHandle* handle) {
	ioHandleLock->lock();
	kiv_os::THandle result = 0;
	auto it = openedHandles.begin();
	//iterate though handles until you find free handle (map is ordered)
	while (it->first == result) {
		//todo does it work though?
		++it;
		++result;
	}
	ioHandleLock->unlock();
	return result;
}

IOHandle* io::getIoHandle(kiv_os::THandle handle) {
	ioHandleLock->lock();
	auto it = openedHandles.find(handle);
	IOHandle* result = nullptr;
	if (it != openedHandles.end()) {
		result = it->second;
	}
	ioHandleLock->unlock();
	return result;
}
void io::removeIoHandle(kiv_os::THandle handle) {
	ioHandleLock->lock();
	auto it = openedHandles.find(handle);
	IOHandle* result = nullptr;
	delete it->second;
	if (it != openedHandles.end()) {
		openedHandles.erase(it);
	}
	ioHandleLock->unlock();
}


void io::Handle_IO(kiv_hal::TRegisters &regs) {
	switch (static_cast<kiv_os::NOS_File_System>(regs.rax.l)) {
		case kiv_os::NOS_File_System::Open_File: {
			OpenIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Write_File: {
			WriteIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Read_File: {
			ReadIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Seek: {
			SeekIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Close_Handle: {
			CloseIOHandle(regs);
			break;
		}
		case kiv_os::NOS_File_System::Delete_File: {
			DeleteFsFile(regs);
			break;
		}
		
		case kiv_os::NOS_File_System::Set_Working_Dir: {
			SetWorkingDirectory(regs);
			break;
		}
		case kiv_os::NOS_File_System::Get_Working_Dir: {
			GetWorkingDirectory(regs);
			break;
		}
		case kiv_os::NOS_File_System::Set_File_Attribute: {
			SetFileAttribute(regs);
			break;
		}
		case kiv_os::NOS_File_System::Get_File_Attribute: {
			GetFileAttribute(regs);
			break;
		}
		case kiv_os::NOS_File_System::Create_Pipe: {
			CreatePipe(regs);
			break;
		}
	}
}

void io::OpenIOHandle(kiv_hal::TRegisters& regs){
	Open_File(regs);
}

void io::WriteIOHandle(kiv_hal::TRegisters& regs){
	//Spravne bychom nyni meli pouzit interni struktury kernelu a zadany handle resolvovat na konkretni objekt, ktery pise na konkretni zarizeni/souboru/roury.
			//Ale protoze je tohle jenom kostra, tak to rovnou biosem posleme na konzoli.
	kiv_hal::TRegisters registers;
	registers.rax.h = static_cast<decltype(registers.rax.h)>(kiv_hal::NVGA_BIOS::Write_String);
	registers.rdx.r = regs.rdi.r;
	registers.rcx = regs.rcx;

	//preklad parametru dokoncen, zavolame sluzbu
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);

	regs.flags.carry |= (registers.rax.r == 0 ? 1 : 0);	//jestli jsme nezapsali zadny znak, tak jiste doslo k nejake chybe
	regs.rax = registers.rcx;	//VGA BIOS nevraci pocet zapsanych znaku, tak predpokladame, ze zapsal vsechny


}

void io::ReadIOHandle(kiv_hal::TRegisters& regs){
	//viz uvodni komentar u Write_File
	regs.rax.r = Read_Line_From_Console(reinterpret_cast<char*>(regs.rdi.r), regs.rcx.r);
}

void io::SeekIOHandle(kiv_hal::TRegisters& regs){
}

void io::CloseIOHandle(kiv_hal::TRegisters& regs){
}

void io::DeleteFsFile(kiv_hal::TRegisters& regs){
}

void io::SetWorkingDirectory(kiv_hal::TRegisters& regs){
	bool result = Set_Working_Dir(regs);
	if (!result) regs.flags.carry = 1;
}
void io::GetWorkingDirectory(kiv_hal::TRegisters& regs) {
	
}
void io::SetFileAttribute(kiv_hal::TRegisters& regs){
}

void io::GetFileAttribute(kiv_hal::TRegisters& regs){
}

void io::CreatePipe(kiv_hal::TRegisters& regs){
	Pipe* pipe = new Pipe(1024);

	PipeIn* in = new PipeIn();
	PipeOut* out = new PipeOut();
	auto* pipeHandles = reinterpret_cast<kiv_os::THandle*>(regs.rdx.r);
	pipeHandles[0] = addIoHandle(in);
	pipeHandles[1] = addIoHandle(out);
}
