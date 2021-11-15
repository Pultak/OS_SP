#pragma once
#include "Process.h"



class ThreadControlBlock{
	std::map<kiv_os::THandle, std::shared_ptr<Process*>> table;

};

