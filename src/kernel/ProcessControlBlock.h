#pragma once
class ProcessControlBlock{

	static std::map<kiv_os::THandle, std::shared_ptr<ProcessFunc::Process*>> table;

};

