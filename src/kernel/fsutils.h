#pragma once
#include <vector>
#include"../api/api.h"
#include"../api/hal.h"
#include <string>

struct directory_item {
	std::string filename = ""; 
	std::string extension = ""; 
	size_t filesize = 0;
	int first_cluster = 0; 

	unsigned char attribute = '0';
};

std::vector<std::string> get_directories(const char* path);

void write_to_fs(int start, std::vector<char> data);

std::vector<unsigned char> read_from_fs(int start, int sectors_count);

std::vector<unsigned char> load_first_table();

std::vector<unsigned char> load_second_table();

int create_folder(const char* name, uint8_t attr, std::vector<unsigned char>& fat_table, std::vector<int>& int_fat_table);

std::vector<int> convert_fat_to_dec(std::vector<unsigned char> fat_table);

directory_item retrieve_item(int start_cluster, std::vector<int> fat_table_dec, std::vector<std::string> path);

std::vector<directory_item> retrieve_folders_from_folder(std::vector<int> fat_table_dec, int working_dir_sector);

std::vector<directory_item> get_dir_items(int num_sectors, std::vector<unsigned char> dir_cont);

std::vector<int> retrieve_sectors_fs(std::vector<int> fat_table_dec, int starting_sector);

int retrieve_free_index(std::vector<int> int_fat_table);

std::vector<unsigned char> num_to_bytes_fs(int target, std::vector<unsigned char> fat_table, int num);

std::vector<unsigned char> dec_to_hex(int start);

void save_fat(std::vector<unsigned char> fat_table);

std::vector<kiv_os::TDir_Entry> get_os_dir_content(size_t sectors, std::vector<unsigned char> clusters, bool is_root);