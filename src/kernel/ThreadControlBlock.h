#pragma once



class ThreadControlBlock{
	std::map<kiv_os::THandle, std::shared_ptr<ProcessFunc::Process*>> table;

};

