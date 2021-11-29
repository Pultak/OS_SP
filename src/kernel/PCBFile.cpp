#include "PCBFile.h"

kiv_os::NOS_Error PCBFileHandle::read(size_t size, char* buffer, size_t& read){
	if (!ProcessUtils::pcb->isUpdated()) {
		size_t processCount = 0;
		//first get all pcb entries
		auto pcbEntries = ProcessUtils::pcb->getAllProcesses(processCount);
		//we can interpet it buffer
		auto result = static_cast<char*>(pcbEntries);
		size_t written = 0;
		//lets write it to reserved file
		file->position = 0;
		write(result, processCount * sizeof(ProcessEntry), written);
		if (written <= 0) {
			return kiv_os::NOS_Error::IO_Error;
		}
		file->position = 0;
	}
	return FileHandle::read(size, buffer, read);
}
