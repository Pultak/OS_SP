#pragma once
#include <process.h>



class ThreadControlBlock{
	std::map<kiv_os::THandle, std::shared_ptr<Process*>> table;

};

