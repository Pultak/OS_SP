#pragma once
#include "FileHandle.h"

FileHandle::FileHandle(VFS* vfs, File file) : vfs(vfs), file(file) {}

kiv_os::NOS_Error FileHandle::seek(size_t new_pos, kiv_os::NFile_Seek position, kiv_os::NFile_Seek op, size_t& res) {
	return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FileHandle::write(char* buffer, size_t size, size_t& written) {
	return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FileHandle::read(size_t size, char* buffer, size_t& read) {
	return kiv_os::NOS_Error::Success;
}

void FileHandle::close() {

}

bool FileHandle::has_attribute(kiv_os::NFile_Attributes attribute) {
	return true;
}
bool FileHandle::is_read_only() {
	return true;
}
bool FileHandle::is_system_file() {
	return true;
}
bool FileHandle::is_directory() {
	return true;
}
bool FileHandle::is_hidden() {
	return true;
}
bool FileHandle::is_volume_id() {
	return true;
}
bool FileHandle::is_archive() {
	return true;
}