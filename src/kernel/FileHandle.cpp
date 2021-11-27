#pragma once
#include "FileHandle.h"
#include <algorithm>

FileHandle::FileHandle(VFS* vfs, File file) : vfs(vfs), file(file) {}

kiv_os::NOS_Error FileHandle::seek(size_t new_pos, kiv_os::NFile_Seek position, kiv_os::NFile_Seek op, size_t& res) {
    if (op == kiv_os::NFile_Seek::Get_Position) {
        res = file.position;
    }
    else if (op == kiv_os::NFile_Seek::Set_Size) {
        if (is_read_only()) {
            return kiv_os::NOS_Error::Permission_Denied;
        }
        file.size = new_pos;
        file.position = new_pos;
    }
    else if (op == kiv_os::NFile_Seek::Set_Position) {
        if (position == kiv_os::NFile_Seek::Beginning) {
            file.position = new_pos;
        }
        else if (position == kiv_os::NFile_Seek::Current) {
            file.position = std::max(file.size, file.position + new_pos);
        }
        else if (position == kiv_os::NFile_Seek::End) {
            file.position = file.size - new_pos;
        }
    }

    return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FileHandle::write(const char* buffer, size_t size, size_t& written) {
    if (is_read_only() || is_directory()) {
        return kiv_os::NOS_Error::Permission_Denied;
    }
    std::vector<char> buf(buffer, buffer + size);

    auto result = vfs->write(file, size, file.position, buf, written);
    file.position += written;
    file.size += written;

    return result;
}

kiv_os::NOS_Error FileHandle::read(size_t size, char* buffer, size_t& read) {
    std::vector<char> out;
    size = std::min(size, file.size - file.position);
    if (size <= 0) {
        return kiv_os::NOS_Error::IO_Error;
    }
    auto result = vfs->read(file, size, file.position, out);
    for (size_t i = 0; i < out.size(); i++) {
        buffer[i] = out.at(i);
    }
    read = out.size();
    file.position += read;

    if (read > 0 && result == kiv_os::NOS_Error::Success) {
        return kiv_os::NOS_Error::Success;
    }
    return result;
}

void FileHandle::close() {

}

bool FileHandle::has_attribute(kiv_os::NFile_Attributes attribute) {
	return (static_cast<uint8_t>(this->file.attributes) & static_cast<uint8_t>(attribute)) != 0;
}
bool FileHandle::is_read_only() {
	return has_attribute(kiv_os::NFile_Attributes::Read_Only);
}
bool FileHandle::is_system_file() {
	return has_attribute(kiv_os::NFile_Attributes::Directory);
}
bool FileHandle::is_directory() {
	return has_attribute(kiv_os::NFile_Attributes::Hidden);
}
bool FileHandle::is_hidden() {
	return has_attribute(kiv_os::NFile_Attributes::System_File);
}
bool FileHandle::is_volume_id() {
	return has_attribute(kiv_os::NFile_Attributes::Volume_ID);
}
bool FileHandle::is_archive() {
	return has_attribute(kiv_os::NFile_Attributes::Archive);
}