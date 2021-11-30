#pragma once
#include "../api/api.h"
#include <string>
#include<vector>
// struktura souboru obsahujici handle (u souboru a slozky cislo sektoru na kterem zacina)
struct File {
    kiv_os::THandle handle;
    uint8_t attributes;
    size_t size;
    size_t position; // aktualni pozice
    char* name; // jmeno souboru (relativni cesta ve filesystemu.
};

class VFS {
public:
	virtual kiv_os::NOS_Error mkdir(const char* pth, uint8_t attr) = 0;

    virtual kiv_os::NOS_Error rmdir(const char* pth) = 0;

    virtual kiv_os::NOS_Error read(File* f, size_t size, size_t offset, std::vector<char>& out) = 0;

    virtual kiv_os::NOS_Error dirread(const char* pth, std::vector<kiv_os::TDir_Entry>& entries) = 0;

	virtual kiv_os::NOS_Error open(const char* pth, kiv_os::NOpen_File flags, uint8_t attributes, File* file) = 0;

    virtual bool file_exist(const char* pth) = 0;

    virtual kiv_os::NOS_Error write(File* f, size_t size, size_t offset, std::vector<char> buffer, size_t& written) = 0;

    virtual std::vector<char> convert_dirs_to_chars(const std::vector<kiv_os::TDir_Entry>& directories) = 0;

    virtual kiv_os::NOS_Error set_file_attribute(const char* name, uint8_t attribute) = 0;

    virtual kiv_os::NOS_Error get_file_attribute(const char* name, uint8_t& out_attribute) = 0;

	~VFS() {};

};