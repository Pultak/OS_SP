#include "ProcessUtils.h"

size_t __stdcall ProcessUtils::defaultSignalHandler(const kiv_hal::TRegisters& regs) {
    auto signal_id = static_cast<kiv_os::NSignal_Id>(regs.rcx.l);

    switch (signal_id) {
        case kiv_os::NSignal_Id::Terminate: {
            //todo kill the system or something idk
            printf("Terminate called!");
            break;
        }
        default: {
            printf("Default signal handler called!");
            break;
        }
    }

    return 0;
};

void ProcessUtils::HandleProcess(kiv_hal::TRegisters& registers, HMODULE user_programs) {

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
        break;
    }
}

void ProcessUtils::clone(kiv_hal::TRegisters& registers, HMODULE userSpaceLib) {
    switch (static_cast<kiv_os::NClone>(registers.rcx.l)) {
    case kiv_os::NClone::Create_Process: {
        //spousteni noveho programu 
        cloneProcess(registers, userSpaceLib);
        break;
    }
    case kiv_os::NClone::Create_Thread: {
        cloneThread(registers);
        break;
    }
    }
}


void ProcessUtils::cloneProcess(kiv_hal::TRegisters& registers, HMODULE userSpaceLib) {
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

        Synchronization::Spinlock* synchLock = new Synchronization::Spinlock(true);
        std::thread t1(processStartPoint, newProcessRegs, progFuncAddress, synchLock);

        auto tHandle = handles::Convert_Native_Handle(t1.get_id(), t1.native_handle(), kiv_os::Invalid_Handle);
        auto thisHandle = handles::getTHandleById(std::this_thread::get_id());
        //is actual process inside pcb?
        Process* thisProcess = pcb->getProcess(thisHandle);
        std::filesystem::path workingDir = thisProcess ? thisProcess->workingDirectory : "C://";

        pcb->AddNewProcess(tHandle, stdIn, stdOut, programName, workingDir);

        // return new process handle
        registers.rax.x = tHandle; 
        registers.flags.carry = 0;
        synchLock->unlock();
        t1.detach();
    }
    else {
        registers.flags.carry = 1;
        registers.rax.x = (uint16_t)kiv_os::NOS_Error::Invalid_Argument;
    }
}
void ProcessUtils::cloneThread(kiv_hal::TRegisters& registers) {
    kiv_os::TThread_Proc progAddr = (kiv_os::TThread_Proc)registers.rdx.r;
    if(progAddr){
        kiv_hal::TRegisters threadRegs{};
        //copy parameters from process
        threadRegs.rdi.r = registers.rdi.r;

        //lock to keep the thread locked until initialization done
        Synchronization::Spinlock* synchLock = new Synchronization::Spinlock(true);
        std::thread t1(ProcessUtils::threadStartPoint, progAddr, threadRegs, synchLock);

        auto parentHandle = handles::getTHandleById(std::this_thread::get_id());
        auto threadHandle = handles::Convert_Native_Handle(t1.get_id(), t1.native_handle(), parentHandle);

        auto parentProcess = pcb->getProcess(parentHandle);
        if (parentProcess) {
            parentProcess->addNewThread(threadHandle);

        }

        registers.rax.x = threadHandle;
        registers.flags.carry = 0;

        synchLock->unlock();
        t1.detach();
    }
    else {
        registers.flags.carry = 1;
        registers.rax.x = (uint16_t)kiv_os::NOS_Error::Invalid_Argument;
    }
}


void ProcessUtils::processStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock) {
    lock->lock();
    delete lock;
    userProgram(registers);

    //after program is finished:

    auto handle = handles::getTHandleById(std::this_thread::get_id());

    //maybe different for thread?
    Process* thisProcess = pcb->getProcess(handle);

    handles::Remove_Handle(handle);

    thisProcess->state = ProcessState::Terminated;
    thisProcess->notifyRemoveListeners();
    
}

void ProcessUtils::threadStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock) {
    lock->lock();
    delete lock;
    userProgram(registers);

    //instructions after program end:
    kiv_os::THandle threadHandle = handles::getTHandleById(std::this_thread::get_id());


    handles::removeHandleById(std::this_thread::get_id(), true);
}


void ProcessUtils::waitFor(kiv_hal::TRegisters& registers) {
    auto* handles = reinterpret_cast<kiv_os::THandle*>(registers.rdx.r);
    const uint8_t handleCount = registers.rcx.l;


    kiv_os::THandle thisHandle = handles::getTHandleById(std::this_thread::get_id());
    if (thisHandle == kiv_os::Invalid_Handle) {
        //todo invalidWaitForRequest(0, handles, thisHandle);
        //thisHandle = 0;
    }
    kiv_os::THandle actualHandle = kiv_os::Invalid_Handle;

    std::vector<SleepListener*> listeners(handleCount, new SleepListener(thisHandle));
    for (int i = 0; i < handleCount; ++i) {
        actualHandle = handles[i];
        if (handles::Resolve_kiv_os_Handle(actualHandle) == INVALID_HANDLE_VALUE) {
            //found invalid handle!
            invalidWaitForRequest(i, handles, thisHandle);
            return;
        }
        else {
            Process* process = pcb->getProcess(actualHandle);
            if (process->state != ProcessState::Terminated) {
                process->addListener(listeners[i]);
            }
            else {
                //process is already dead -> no reason to keep listener locked
                listeners[i]->lock->unlock();
            }
        }
    }
    int index = 0;
    for (; index < handleCount; index++) {
        //wait until notified
        listeners[index]->lock->lock();
        //reference to listener from the process is already removed
        delete listeners[index];
    }
    listeners.clear();
    //return last index of handle
    registers.rax.l = index;

}

void ProcessUtils::invalidWaitForRequest(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle) {
    for (int i = 0; i < alreadyDone; i++) {
        // load a handle
        auto handle = handles[i];
        Process* process = pcb->getProcess(handle);
        if (process) {
            process->removeListener(thisHandle);
        }
        else {
            //process already finished
        }
    }

    //todo set flags?


}


void ProcessUtils::exit(kiv_hal::TRegisters& registers) {
    //todoo process get handle
    kiv_os::THandle pHandle = kiv_os::Invalid_Handle;
    if (pHandle != kiv_os::Invalid_Handle) {
        Process* exitingProcess = pcb->getProcess(pHandle);
        if (exitingProcess != nullptr) {
            auto exitCode = static_cast<kiv_os::NOS_Error>(registers.rcx.x);
            exitingProcess->exitCode = exitCode;
            exitingProcess->state = ProcessState::Terminated;
        }
        else {
            //todo what now? Process is missing  
        }

    }
    else {
        //todo missing handle
    }

}

void ProcessUtils::readExitCode(kiv_hal::TRegisters& registers) {
    auto resultExitCode = kiv_os::NOS_Error::Unknown_Error;
    kiv_os::THandle handle = static_cast<kiv_os::THandle>(registers.rdx.x);

    //is handle of active thread?
    if (handle != kiv_os::Invalid_Handle) {
        Process* process = pcb->getProcess(handle);
        if (process && process->state == ProcessState::Terminated) {
            resultExitCode = process->exitCode;
            pcb->removeProcess(handle);
        }
        else {
            //todo process missing or not yet finished
        }
    }
    else {
        //todo handle missing
    }
    registers.rcx.x = static_cast<uint16_t>(resultExitCode);
}


void ProcessUtils::shutdown() {

    pcb->signalProcesses(kiv_os::NSignal_Id::Terminate);

    //todo jeste neco? zavrit files?


}

void ProcessUtils::registerSignalHandler(kiv_hal::TRegisters& registers) {
    //todo process get handle
    auto handle = handles::getTHandleById(std::this_thread::get_id());

    if (handle != kiv_os::Invalid_Handle) {
        Process* actualProcess = pcb->getProcess(handle);
        if (actualProcess == nullptr) {
            //todo missing process
        }
        else {
            auto signal = static_cast<kiv_os::NSignal_Id>(registers.rcx.l);
            //read program address of the signal handler
            auto progAddress = reinterpret_cast<kiv_os::TThread_Proc>(registers.rdx.r);
            if (progAddress == nullptr)progAddress = defaultSignalHandler;
            actualProcess->signalHandlers[signal] = progAddress;
        }
    }
    else {
        //todo missing handle
    }
}

