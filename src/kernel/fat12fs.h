#pragma once
#include "vfs.h"
#include "fsutils.h"

class FAT: public VFS{
public:

	/*
	* konstruktor tridy, uklada si cislo disku pro zapis a cteni a parametry.
	*/
	FAT(uint8_t disk, kiv_hal::TDrive_Parameters params);

	/*
	* metoda pripravi a ulozi novou slozku
	*/
	kiv_os::NOS_Error mkdir(const char* pth, uint8_t attr) override;

	/*
	* metoda vymaze slozku na zadane ceste
	*/
	kiv_os::NOS_Error rmdir(const char* pth) override;

	/*
	* metoda precte obsah slozky na zadane ceste a ulozi jej do entries
	*/
	kiv_os::NOS_Error dirread(const char* pth, std::vector<kiv_os::TDir_Entry>& entries) override;

	/*
	* metoda otevre soubor na zadane ceste a vrati jeho udaje ve strukture File
	*/
	kiv_os::NOS_Error open(const char* pth, kiv_os::NOpen_File flags, uint8_t attributes, File* file) override;

	/*
	* metoda overi zda-li na zadane ceste existuje soubor
	*/
	bool file_exist(const char* pth) override;

	/*
	* metoda zapise do souboru predany buffer
	*/
	kiv_os::NOS_Error write(File* f, size_t size, size_t offset, std::vector<char> buffer, size_t& written) override;

	/*
	* metoda precte ze souboru nebo slozky data o zadane velikosti a ulozi je do vektoru charu out
	*/
	kiv_os::NOS_Error read(File* f, size_t size, size_t offset, std::vector<char>& out) override;

	/*
	* metoda vezme predany vektor TDir_Entry a prevede jej do vektoru charu pro vypis
	*/
	std::vector<char> convert_dirs_to_chars(const std::vector<kiv_os::TDir_Entry>& directories) override;

	/*
	* metoda prenastavi atribut souboru na zadane ceste
	*/
	kiv_os::NOS_Error set_file_attribute(const char* name, uint8_t attribute) override;

	/*
	* metoda ziska atribut soubopru na predane ceste
	*/
	kiv_os::NOS_Error get_file_attribute(const char* name, uint8_t& out_attribute) override;

	/*
	* destruktor tridy
	*/
	~FAT() {}

private:
	/*
	* cislo disku
	*/
	uint8_t disk;

	/*
	* parametry disku
	*/
	kiv_hal::TDrive_Parameters parameters;

};