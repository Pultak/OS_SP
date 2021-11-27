#include "VGAHandle.h"

kiv_os::NOS_Error VGAHandle::write(const char* buffer, const size_t size, size_t& written){
	//Spravne bychom nyni meli pouzit interni struktury kernelu a zadany handle resolvovat na konkretni objekt, ktery pise na konkretni zarizeni/souboru/roury.
			//Ale protoze je tohle jenom kostra, tak to rovnou biosem posleme na konzoli.
	kiv_hal::TRegisters registers;
	registers.rax.h = static_cast<uint8_t>(kiv_hal::NVGA_BIOS::Write_String);
	registers.rdx.r = reinterpret_cast<uint64_t>(buffer);
	registers.rcx.r = size;

	//preklad parametru dokoncen, zavolame sluzbu
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
	kiv_os::NOS_Error returnCode = (registers.rax.r == 0 ? kiv_os::NOS_Error::IO_Error : kiv_os::NOS_Error::Success);	//jestli jsme nezapsali zadny znak, tak jiste doslo k nejake chybe
	//regs.rax = registers.rcx;	
	//VGA BIOS nevraci pocet zapsanych znaku, tak predpokladame, ze zapsal vsechny
	written = size;
	return returnCode;
}

kiv_os::NOS_Error VGAHandle::read(const size_t size, char* buffer, size_t& read){
	return kiv_os::NOS_Error::IO_Error;
}
void VGAHandle::close() {}
