#pragma once

#include "..\api\api.h"
#include <map>
#include "IOHandle.h"
#include <set>
#include "kernel.h"
#include "dir.h"
#include "filesystems.h"
#include "handles.h"
#include "PipeUtils.h"

namespace io{
	//std::set<int> used;


	kiv_os::THandle addIoHandle(IOHandle* handle);
	IOHandle* getIoHandle(kiv_os::THandle handle);
	void removeIoHandle(kiv_os::THandle handle);

	size_t Read_Line_From_Console(char* buffer, const size_t buffer_size);

	void Handle_IO(kiv_hal::TRegisters& regs);

	void OpenIOHandle(kiv_hal::TRegisters& regs);
	void WriteIOHandle(kiv_hal::TRegisters& regs);
	void ReadIOHandle(kiv_hal::TRegisters& regs);
	void SeekIOHandle(kiv_hal::TRegisters& regs);
	void CloseIOHandle(kiv_hal::TRegisters& regs);
	void DeleteFsFile(kiv_hal::TRegisters& regs);
	void SetWorkingDirectory(kiv_hal::TRegisters& regs);
	void GetWorkingDirectory(kiv_hal::TRegisters& regs);
	void SetFileAttribute(kiv_hal::TRegisters& regs);
	void GetFileAttribute(kiv_hal::TRegisters& regs);
	void CreatePipe(kiv_hal::TRegisters& regs);

}


