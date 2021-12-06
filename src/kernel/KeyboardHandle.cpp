#include "KeyboardHandle.h"


bool readClosed = false;


kiv_os::NOS_Error KeyboardHandle::write(const char* buffer, const size_t size, size_t& written){
	return kiv_os::NOS_Error::IO_Error;
}

kiv_os::NOS_Error KeyboardHandle::read(const size_t size, char* buffer, size_t& read){
	kiv_hal::TRegisters registers;
	if (readClosed) {
		if (size > 0) {
			buffer[0] = static_cast<char>(kiv_hal::NControl_Codes::EOT);
			read = 1;
		}
		kiv_os::NOS_Error::Success;
	}
	size_t pos = 0;
	while (pos < size) {

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
			if (pos > 0) {
				--pos;
				--read;
			}

			registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_Control_Char);
			registers.rdx.l = ch;
			kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
		}
										break;

		case kiv_hal::NControl_Codes::LF:  break;	//jenom pohltime, ale necteme
		case kiv_hal::NControl_Codes::NUL:			//chyba cteni?
		//case kiv_hal::NControl_Codes::EOT:			//konec textu
		case kiv_hal::NControl_Codes::CR:  return kiv_os::NOS_Error::Success;	//docetli jsme az po Enter


		default: buffer[pos] = ch;
			++pos;
			//add one byte to the readCount
			++read;
			registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NVGA_BIOS::Write_String);
			registers.rdx.r = reinterpret_cast<decltype(registers.rdx.r)>(&ch);
			registers.rcx.r = 1;
			if (static_cast<kiv_hal::NControl_Codes>(ch) == kiv_hal::NControl_Codes::ETX ||
				static_cast<kiv_hal::NControl_Codes>(ch) == kiv_hal::NControl_Codes::EOT) {
				read = pos;
				return kiv_os::NOS_Error::Success;
			}
			else {
				kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
			}
			//kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::VGA_BIOS, registers);
			break;
		}
	}
	read = pos;
	return kiv_os::NOS_Error::Success;
}

void KeyboardHandle::close() {
	kiv_hal::TRegisters registers;
	registers.flags.non_zero = 0;
	//read char
	registers.rax.l = static_cast<uint8_t>(kiv_hal::NControl_Codes::EOT);
	registers.rax.h = static_cast<decltype(registers.rax.l)>(kiv_hal::NKeyboard::Write_Char);
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Keyboard, registers);
	readClosed = true;
}
