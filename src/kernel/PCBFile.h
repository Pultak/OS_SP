#pragma once

#include "FileHandle.h"
#include "ProcessUtils.h"

class PCBFileHandle : public FileHandle {
public:
	PCBFileHandle(VFS* vfs, File* file) : FileHandle(vfs, file) {

	}

	kiv_os::NOS_Error read(size_t size, char* buffer, size_t& read) override;

};