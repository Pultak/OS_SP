#include "ProcessUtils.h"

size_t __stdcall ProcessUtils::defaultSignalHandler(const kiv_hal::TRegisters& regs) {
    auto signal_id = static_cast<kiv_os::NSignal_Id>(regs.rcx.l);

    switch (signal_id) {
        case kiv_os::NSignal_Id::Terminate: {
            //kill the system or something idk
            shutdown();
            break;
        }
        default: {
            //Default signal handler called
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
        registers.flags.carry = 1;
        registers.rax.r = static_cast<uint64_t>(kiv_os::NOS_Error::Invalid_Argument);
        break;
    }
}

void ProcessUtils::clone(kiv_hal::TRegisters& registers, HMODULE userSpaceLib) {
    switch (static_cast<kiv_os::NClone>(registers.rcx.l)) {
        case kiv_os::NClone::Create_Process: {
            cloneProcess(registers, userSpaceLib);
            break;
        }
        case kiv_os::NClone::Create_Thread: {
            cloneThread(registers);
            break;
        }
        default:
            registers.flags.carry = 1;
            registers.rax.r = static_cast<uint64_t>(kiv_os::NOS_Error::Invalid_Argument);
            return;
    }
}


void ProcessUtils::cloneProcess(kiv_hal::TRegisters& registers, HMODULE userSpaceLib) {
    auto programName = (char*)registers.rdx.r;
    auto progFuncAddress = (kiv_os::TThread_Proc)GetProcAddress(userSpaceLib, programName);

    //does the user function even exist?
    if (progFuncAddress) {
        //get the passed input and output handles
        kiv_os::THandle stdIn = (registers.rbx.e >> 16) & 0xFFFF;
        kiv_os::THandle stdOut = registers.rbx.e & 0xFFFF;

        kiv_hal::TRegisters newProcessRegs{};
        newProcessRegs.rax.x = stdIn;
        newProcessRegs.rbx.x = stdOut;
        //copy program args
        newProcessRegs.rdi.r = registers.rdi.r;

        Synchronization::Spinlock* synchLock = new Synchronization::Spinlock(true);
        std::thread t1(processStartPoint, newProcessRegs, progFuncAddress, synchLock);

        //init the handle of the new process
        auto tHandle = handles::Convert_Native_Handle(t1.get_id(), t1.native_handle(), kiv_os::Invalid_Handle);
        auto thisHandle = handles::getTHandleById(std::this_thread::get_id());
        //is actual process inside pcb?
        Process* thisProcess = pcb->getProcess(thisHandle);
        std::filesystem::path workingDir = thisProcess ? thisProcess->workingDirectory : "C:\\";

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

    //does the user function even exist?
    if(progAddr){
        kiv_hal::TRegisters threadRegs{};
        //copy parameters from process
        threadRegs.rdi.r = registers.rdi.r;

        //lock to keep the thread locked until initialization done
        Synchronization::Spinlock* synchLock = new Synchronization::Spinlock(true);
        std::thread t1(ProcessUtils::threadStartPoint, threadRegs, progAddr, synchLock);

        auto parentHandle = handles::getTHandleById(std::this_thread::get_id());
        //create new handle with the reference to parent
        auto threadHandle = handles::Convert_Native_Handle(t1.get_id(), t1.native_handle(), parentHandle);

        auto parentProcess = pcb->getProcess(parentHandle);
        if (parentProcess) {
            parentProcess->addNewThread(threadHandle);
        }
        //return new handle to the userspace
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

	const char* arg = reinterpret_cast<char*>(registers.rdi.r);


    userProgram(registers);

    //after program is finished:
    auto handle = handles::getTHandleById(std::this_thread::get_id());

    Process* thisProcess = pcb->getProcess(handle);
    handles::Remove_Handle(handle);

    thisProcess->state = ProcessState::Terminated;
    thisProcess->notifyRemoveListeners(handle);
}

void ProcessUtils::threadStartPoint(kiv_hal::TRegisters& registers, kiv_os::TThread_Proc userProgram, Synchronization::Spinlock* lock) {
    lock->lock();
    delete lock;
    userProgram(registers);

    //instructions after program end:
    kiv_os::THandle threadHandle = handles::getTHandleById(std::this_thread::get_id());

    auto parentHandle = handles::getParentTHandleById(std::this_thread::get_id());
    handles::removeHandleById(std::this_thread::get_id(), true);
    if (parentHandle != kiv_os::Invalid_Handle) {
        //get parent process to remove thread from process tcb
        auto parentProcess = pcb->getProcess(parentHandle);
        if (parentProcess){
            auto thread = parentProcess->getThread(threadHandle);
            if (thread) {
                thread->notifyRemoveListeners(threadHandle);
            }
            parentProcess->removeThread(threadHandle);
        }
    }
}


void ProcessUtils::waitFor(kiv_hal::TRegisters& registers) {
    auto* handles = reinterpret_cast<kiv_os::THandle*>(registers.rdx.r);
    const uint8_t handleCount = registers.rcx.l;


    kiv_os::THandle thisHandle = handles::getTHandleById(std::this_thread::get_id());
    kiv_os::THandle actualHandle = kiv_os::Invalid_Handle;

    auto listener = new SleepListener(thisHandle); 
    int index = 0;
    for (index = 0; index < handleCount; ++index) {
        actualHandle = handles[index];
        if (handles::Resolve_kiv_os_Handle(actualHandle) == INVALID_HANDLE_VALUE) {
            //found invalid handle!
            removeAssignedListener(index, handles, thisHandle);
            delete listener;
            registers.rax.l = index;
            return;
        }
        else {
            Process* process = pcb->getProcess(actualHandle);
            if (process->state != ProcessState::Terminated) {
                process->addListener(listener);
            }
            else {
                //process is already dead -> no reason to keep listener locked
                removeAssignedListener(index, handles, thisHandle);
                delete listener;
                registers.rax.l = index;
                return;
            }
        }
    }
    listener->lock->lock();
    auto notifiedHandle = listener->notifierHandle;
    removeAssignedListener(index, handles, thisHandle);
    delete listener;

    index = -1;
    //lets find notifier handle in the handle array
    for (int i = 0; i < handleCount; ++i) {
        if (handles[i] == notifiedHandle) {
            index = i;
        }
    }
    //return index of notifier handle
    registers.rax.l = index;

}
void ProcessUtils::removeAssignedListener(const int alreadyDone, const kiv_os::THandle* handles, const kiv_os::THandle thisHandle) {
    for (int i = 0; i < alreadyDone; i++) {
        // load a handle
        auto handle = handles[i];
        Process* process = pcb->getProcess(handle);
        if (process) {
            process->removeListener(thisHandle);
        }
    }
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
    }else {
        //missing handle
        registers.flags.carry = 1;
        registers.rcx.x = static_cast<uint16_t>(kiv_os::NOS_Error::Invalid_Argument);
    }
}

void ProcessUtils::readExitCode(kiv_hal::TRegisters& registers) {
    auto resultExitCode = kiv_os::NOS_Error::Unknown_Error;
    kiv_os::THandle handle = static_cast<kiv_os::THandle>(registers.rdx.x);

    //is handle of active thread?
    if (handle != kiv_os::Invalid_Handle) {
        Process* process = pcb->getProcess(handle);
        if (process && process->state == ProcessState::Terminated) {
            //process already terminated -> OK
            resultExitCode = process->exitCode;
            pcb->removeProcess(handle);
        }
        else {
            //process missing or not yet finished or the handle is of thread
            registers.flags.carry = 1;
        }
    }
    else {
        resultExitCode = kiv_os::NOS_Error::Invalid_Argument;
        registers.flags.carry = 1;
    }
    registers.rcx.x = static_cast<uint16_t>(resultExitCode);
}


void ProcessUtils::shutdown() {

    pcb->notifyAllListeners();
    pcb->signalProcesses(kiv_os::NSignal_Id::Terminate);

    //todo jeste neco? zavrit files?

}

void ProcessUtils::registerSignalHandler(kiv_hal::TRegisters& registers) {
    //todo process get handle
    auto handle = handles::getTHandleById(std::this_thread::get_id());

    if (handle != kiv_os::Invalid_Handle) {
        Process* actualProcess = pcb->getProcess(handle);
        if (actualProcess) {
            auto signal = static_cast<kiv_os::NSignal_Id>(registers.rcx.l);
            //read program address of the signal handler
            auto progAddress = reinterpret_cast<kiv_os::TThread_Proc>(registers.rdx.r);
            if (progAddress == nullptr)progAddress = defaultSignalHandler;
            actualProcess->signalHandlers[signal] = progAddress;
        }else{
            //missing process
            registers.flags.carry = 1;
            registers.rcx.x = static_cast<uint16_t>(kiv_os::NOS_Error::Unknown_Error);
        }
    }else {
        //missing handle
        registers.flags.carry = 1;
        registers.rcx.x = static_cast<uint16_t>(kiv_os::NOS_Error::Invalid_Argument);
    }
}

