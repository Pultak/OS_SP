#include "Process.h"

void Process::Handle_Process(kiv_hal::TRegisters& registers, HMODULE user_programs){

	switch (static_cast<kiv_os::NOS_Process>(registers.rax.l)) {
        case kiv_os::NOS_Process::Clone: 
            clone(registers, user_programs);
            break;
        case kiv_os::NOS_Process::Wait_For: 
            wait_for(registers);
            break;
        case kiv_os::NOS_Process::Exit: 
            exit(registers);
            break;
        case kiv_os::NOS_Process::Read_Exit_Code: 
            read_exit_code(registers);
            break;
        case kiv_os::NOS_Process::Shutdown: 
            shutdown();
            break;
        case kiv_os::NOS_Process::Register_Signal_Handler: 
            register_signal_handler(registers);
            break;
        default:
            //TODO what now?
            std::cout << "Wtf u tryin to do?" << std::endl;
            break;
	}
}

void Process::clone(kiv_hal::TRegisters& registers, HMODULE user_programs){


}

void Process::wait_for(kiv_hal::TRegisters& registers){

}

void Process::exit(kiv_hal::TRegisters& registers){

}

void Process::exit(kiv_os::THandle handle, kiv_os::NOS_Error exit_code){
}

void Process::read_exit_code(kiv_hal::TRegisters& registers){
}

void Process::register_signal_handler(kiv_hal::TRegisters& registers){
}

void Process::signal_all_processes(kiv_os::NSignal_Id signal){
}

void Process::signal(kiv_os::NSignal_Id signal_id, Process* process){
}

void Process::shutdown(){
}
