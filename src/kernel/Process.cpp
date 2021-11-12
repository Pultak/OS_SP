#include "Process.h"
#include <thread>
#include "handles.h"

void Process::HandleProcess(kiv_hal::TRegisters& registers, HMODULE user_programs){

	switch (static_cast<kiv_os::NOS_Process>(registers.rax.l)) {
        case kiv_os::NOS_Process::Clone: 
            clone(registers, user_programs);
            break;
        case kiv_os::NOS_Process::Wait_For: 
            waitFor(registers);
            break;
        case kiv_os::NOS_Process::Exit: 
            exit(registers);
            break;
        case kiv_os::NOS_Process::Read_Exit_Code: 
            readExitCode(registers);
            break;
        case kiv_os::NOS_Process::Shutdown: 
            shutdown();
            break;
        case kiv_os::NOS_Process::Register_Signal_Handler: 
            registerSignalHandler(registers);
            break;
        default:
            //TODO what now?
            std::cout << "Wtf u tryin to do?" << std::endl;
            break;
	}
}

void Process::clone(kiv_hal::TRegisters& registers, HMODULE userSpaceLib){
    switch (static_cast<kiv_os::NClone>(registers.rcx.l)) {
    case kiv_os::NClone::Create_Process: {
        //spousteni noveho programu 
        cloneProcess(registers, userSpaceLib);
        break; 
        }
    case kiv_os::NClone::Create_Thread: {
        //todo usefull?
        break;
        }
    }
}


void Process::cloneProcess(kiv_hal::TRegisters& registers, HMODULE userSpaceLib) {
    auto programName = (char*)registers.rdx.r;
    auto progFuncAddress = (kiv_os::TThread_Proc)GetProcAddress(userSpaceLib, programName);

    //does this function even exist?
    if (progFuncAddress) {
        kiv_os::THandle stdIn = (registers.rbx.e >> 16) & 0xFFFF;
        kiv_os::THandle stdOut = registers.rbx.e & 0xFFFF;

        kiv_hal::TRegisters newProcessRegs{};
        newProcessRegs.rax.x = stdIn;
        newProcessRegs.rbx.x = stdOut;
        //copy program args
        newProcessRegs.rdi.r = registers.rdi.r;

        std::thread t1(processStartPoint, newProcessRegs, progFuncAddress);
        auto tHandle = handles::Convert_Native_Handle(t1.native_handle());

        auto thisHandle = handles::getTHandleById(std::this_thread::get_id());
        //is actual process inside pcb?
        auto thisProcess = pcb->getProcess(thisHandle);
        std::filesystem::path workingDir = thisProcess ? thisProcess->workingDirectory : "/";
       
        pcb->AddNewProcess(tHandle, stdIn, stdOut, programName, workingDir);

        // return new process handle
        registers.rax.x = tHandle;
        t1.detach();
    }
    else {
        registers.flags.carry = 1;
        registers.rax.x = (uint16_t)kiv_os::NOS_Error::Invalid_Argument;
    }
}

void Process::processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram ){
    userProgram(registers);

    //after program is finished:

    auto handle = handles::getTHandleById(std::this_thread::get_id());

    //maybe different for thread?
    auto thisProcess = pcb->getProcess(handle);

    handles::Remove_Handle(handle);
    

    for (auto const& listener : thisProcess->listeners) {
        //wake up the slave
        listener->lock->unlock();
    }
    thisProcess->listeners.clear();
    thisProcess->state = ProcessState::Terminated;
}


void Process::waitFor(kiv_hal::TRegisters& registers){
    auto* handles = reinterpret_cast<kiv_os::THandle*>(registers.rdx.r);
    const uint8_t handleCount = registers.rcx.l;

    
    kiv_os::THandle thisHandle = handles::getTHandleById(std::this_thread::get_id());
    if (thisHandle == kiv_os::Invalid_Handle) {
        invalidWaitForRequest(0, handles, thisHandle);
    }
    kiv_os::THandle actualHandle = kiv_os::Invalid_Handle;
    //todo move this to be faster?
    std::vector listeners(handleCount, std::make_unique<SleepListener>(new SleepListener(thisHandle)));
    for (int i = 0; i < handleCount; ++i) {
        actualHandle = handles[i];
        if (handles::Resolve_kiv_os_Handle(actualHandle) == INVALID_HANDLE_VALUE) {
            //found invalid handle!
            invalidWaitForRequest(i, handles, thisHandle);
            return;
        }
        else {
            auto process = pcb->getProcess(actualHandle);
            process->listeners.push_back(listeners[i]);
        }
    }




}

void Process::invalidWaitForRequest(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle) {
    for (int i = 0; i < alreadyDone; i++) {
        // load a handle
        auto handle = handles[i];
        auto process = pcb->getProcess(handle);
        if (process) {
            //todo synchronization needed?
            
            auto i = process->listeners.begin();
            while (i != process->listeners.end()) {
                //remove all inserted listeners
                if ((*i)->sleeperHandle == thisHandle) {
                    process->listeners.erase(i++);
                }
            }
        }
        else {
            //process already finished
        }
    }

    //todo set flags?


}


void Process::exit(kiv_hal::TRegisters& registers){
    //todoo process get handle
    kiv_os::THandle pHandle = kiv_os::Invalid_Handle;
    if (pHandle != kiv_os::Invalid_Handle) {
        auto exitingProcess = pcb->getProcess(pHandle);
        if (exitingProcess != nullptr) {
            auto exitCode = static_cast<kiv_os::NOS_Error>(registers.rcx.x);
            exitingProcess->exitCode = exitCode;
            exitingProcess->state = ProcessState::Terminated;
        }
        else {
            //todo what da process doin
        }

    }
    else {
        //todo 
    }
}

void Process::readExitCode(kiv_hal::TRegisters& registers){
    
    kiv_os::THandle handle = registers.rdx.x;
    
    //handle of active thread?
    if (handle != kiv_os::Invalid_Handle) {
        auto process = pcb->getProcess(handle);
        if (process) {
            registers.rcx.x = static_cast<uint16_t>(process->exitCode);
            //todo remove if already terminated
        }
    }




}


void Process::shutdown(){

    pcb->signalProcesses(kiv_os::NSignal_Id::Terminate);

    //todo jeste neco? File?

}

void Process::registerSignalHandler(kiv_hal::TRegisters& registers){



    //todo process get handle
    kiv_os::THandle handle = kiv_os::Invalid_Handle;

    if (handle != kiv_os::Invalid_Handle) {
        auto actualProcess = pcb->getProcess(handle);
        if (actualProcess == nullptr) {
            //todo what now?
        }else {
            auto signal = static_cast<kiv_os::NSignal_Id>(registers.rcx.l);
            auto progAddress = reinterpret_cast<kiv_os::TThread_Proc>(registers.rdx.r);
            if (progAddress == nullptr)progAddress = default_signal_handler;
            actualProcess->signal_handlers[signal] = progAddress;
        }
    }
    else {
        //todo
    }
}

