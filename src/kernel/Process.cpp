
#include <thread>
#include "handles.h"
#include "Process.h"

void Process::addNewThread(kiv_os::THandle threadHandle){
	tcbLock->lock();
	auto newThread = std::make_unique<Thread>(new Thread(threadHandle, handle));
	tcb.insert(std::make_pair(threadHandle, newThread));
	tcbLock->unlock();
}


