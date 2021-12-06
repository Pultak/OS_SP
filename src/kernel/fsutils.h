#pragma once

#define SECTOR_SIZE 512

#include <vector>
#include"../api/api.h"
#include"../api/hal.h"
#include <string>
#include <memory>

struct directory_item {
	std::string filename = ""; //nazev slozky/souboru
	std::string extension = ""; // pripona slozky/souboru
	size_t filesize = 0; // velikost souboru/slozky
	int first_cluster = 0; //prvni cluster na kterem zacinaji data
	unsigned char attribute = '0'; // stribut souboru/slozky
};

/*
* rozdeli vstupni cestu path na individualni slozky a vrati je ve forme vectoru stringu. 
*/
std::vector<std::string> get_directories(const char* path);

/*
* zapise na sektor dany vstupnim argumentem start data z predaneho vectoru data.
*/
void write_to_fs(int start, std::vector<char> data);

/*
* precte z disku data zacinajici na sektoru start. sectors_count urcuje pocet sektoru ktere se maji precist
*/
std::vector<unsigned char> read_from_fs(int start, int sectors_count);

/*
* metoda nacte obsah prvni fat tabulky do vystupniho vektoru
*/
std::vector<unsigned char> load_first_table();

/*
* metoda nacte obsah prvni fat tabulky do vystupniho vektoru
*/
std::vector<unsigned char> load_second_table();

/*
* metoda vytvori na disku novou polozku directory_item slozky na predane ceste. 
*/
int create_folder(const char* name, uint8_t attr, std::vector<unsigned char>& fat_table, std::vector<int>& int_fat_table);

/*
* vrati obsah fat tabulky v podobe celych cisel.
*/
std::vector<int> convert_fat_to_dec(std::vector<unsigned char> fat_table);

/*
* metoda vrati directory_item na zadane ceste. zacne hledat ve slozce dle pocinajici na start_cluster clusteru.
*/
directory_item retrieve_item(int start_cluster, std::vector<int> fat_table_dec, std::vector<std::string> path);

/*
* metoda vraci vsechny directory_item struktury ze slozky pocinajici na working_dir_sector clusteru.
*/
std::vector<directory_item> retrieve_folders_from_folder(std::vector<int> fat_table_dec, int working_dir_sector);

/*
* metoda prevede vector unsigned charu na directory item struktury.
*/
std::vector<directory_item> get_dir_items(int num_sectors, std::vector<unsigned char> dir_cont);

/*
* vraci vector s cisli clusteru nalezici souboru/slozce zacinajici na clusteru starting_cluster.
*/
std::vector<int> retrieve_sectors_fs(std::vector<int> fat_table_dec, int starting_sector);

/*
* vraci prvni volny index clusteru z fat tabulky.
*/
int retrieve_free_index(std::vector<int> int_fat_table);

/*
* metoda prevede vstupni cislo na vector charu aby bylo mozne ulozit cislo do fat tabulky
*/
std::vector<unsigned char> num_to_bytes_fs(int target, std::vector<unsigned char> fat_table, int num);

/*
* prevede int cislo to podoby char podoby.
*/
std::vector<unsigned char> dec_to_hex(int start);

/*
* prevede size_t cislo do char podoby.
*/
std::vector<unsigned char> dec_t_to_hex(size_t val);

/*
* ulozi predanou fat_tabulku na disk.
*/
void save_fat(std::vector<unsigned char> fat_table);

/*
* metoda prevede vstupni vector charu do podoby TDir_Entry struktur.
*/
std::vector<kiv_os::TDir_Entry> get_os_dir_content(size_t sectors, std::vector<unsigned char> clusters, bool is_root);

/*
* metoda zapise na dany index . a ..
*/
void write_folder_into_fs(int index, int upper_index);

/*
* metoda alokuje novy cluster slozce/souboru zacinajiicm na start sectoru. vraci -1 pokud jiz dalsi volny cluster neni.
*/
int aloc_cluster(int start, std::vector<int>& int_fat_table, std::vector<unsigned char>& fat_table);

/*
* prevede dva chary reprezentujici bajt na hodnotu bajtu.
*/
unsigned char char_to_hex(char charar[2]);

/*
* metoda kontroluje validitu jmena souboru.
*/
bool file_name_val(const char* name);

/*
* metoda kontroluje validitu jmena slozky.
*/
bool folder_name_val(const char* name);

/*
* metoda vytvori ve filesystemu novy soubor na zadane ceste path.
*/
int create_file(const char* path, uint8_t attr, std::vector<unsigned char>& fat_table, std::vector<int>& int_fat_table);

/*
* metoda zaktualizuje velikost souboru na zadane ceste.
*/
void update_file_size(const char* path, size_t offset, size_t org_size, size_t new_bytes_size, std::vector<int> int_fat_table);

