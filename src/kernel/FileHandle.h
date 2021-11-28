#pragma once
#include "vfs.h"
#include "IOHandle.h"

class FileHandle : public IOHandle {
public:
	explicit FileHandle(VFS* vfs, File* file);

	kiv_os::NOS_Error write(const char* buffer, const size_t size, size_t& written) override;
	kiv_os::NOS_Error read(size_t size, char* buffer, size_t& read) override;

	kiv_os::NOS_Error seek(size_t new_pos, kiv_os::NFile_Seek position, kiv_os::NFile_Seek op, size_t& res);

	void close() override;
	~FileHandle() {
		delete file;
	}

private:

	File* file;	
	VFS* vfs;
	bool has_attribute(kiv_os::NFile_Attributes attribute);
	bool is_read_only();
	bool is_system_file();
	bool is_directory();
	bool is_hidden();
	bool is_volume_id();
	bool is_archive();
};