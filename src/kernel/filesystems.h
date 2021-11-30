#pragma once
#include<map>
#include<string>
#include<filesystem>
#include<memory>
#include<iostream>
#include "vfs.h"
#include "IOHandle.h"
#include "FileHandle.h"
#include "PCBFile.h"

namespace filesystems {
	/*
	* pridani C:\ filesystemu
	*/
	void InitFilesystems();

	/*
	* srovnani aktualniho adresare procesu a zadane cesty. vraci absolutni cestu ve filesystemu
	*/
	void parse_path(const char* abs_path, const char* rel_path, std::string& result);

	/*
	* prida novy filesystem do mapy
	*/
	void Add_To_Filesystems(const std::string& name, VFS* vfs);

	/*
	* overi zda-li v mape existuje filesystem podle zadane absolutni cesty a vrati ho
	*/
	VFS* Filesystem_exists(std::filesystem::path path);

	/*
	* pokusi se ziskat filesystem z mapy podle jmena
	*/
	VFS* Get_Filesystem(const std::string& file_name);

	/*
	* otvira handle slozky/souboru 
	*/
	IOHandle* Open_File(const char* input_file_name, kiv_os::NOpen_File flags, uint8_t attributes, kiv_os::NOS_Error& error);

}
