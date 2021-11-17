#include "fsutils.h"
#include "../api/api.h"
#include "../api/hal.h"

std::vector<std::string> get_directories(const char* path){
	std::vector<std::string> directories;
	std::string pth = path;
	std::string delim = "\\";
	size_t pos = 0;
	std::string dir;
	while ((pos = pth.find(delim)) != std::string::npos) {
		dir = pth.substr(0, pos);
		directories.push_back(dir);
		pth.erase(0, pos + delim.length());
	}
	directories.push_back(pth);
	return directories;
}

std::vector<unsigned char> load_first_table() {
	std::vector<unsigned char> fat_table_content;

	kiv_hal::TRegisters read_reg;
	kiv_hal::TDisk_Address_Packet read_packet;
	
	auto s = malloc(512 * 9);
	read_packet.count = 9;
	read_packet.sectors = (void*)s;
	read_packet.lba_index = 1;

	read_reg.rdx.l = 129;
	read_reg.rax.h = static_cast<decltype(read_reg.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_reg.rdi.r = reinterpret_cast<decltype(read_reg.rdi.r)>(&read_packet);
	
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_reg);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);
	for (int i = 0; i < (512 * 9); i++) {
		fat_table_content.push_back(buffer[i]);
	}

	free(s);
	return fat_table_content;
}

std::vector<unsigned char> load_second_table() {
	std::vector<unsigned char> fat_table_content;

	kiv_hal::TRegisters read_reg;
	kiv_hal::TDisk_Address_Packet read_packet;

	auto s = malloc(512 * 9);
	read_packet.count = 9;
	read_packet.sectors = (void*)s;
	read_packet.lba_index = 10;

	read_reg.rdx.l = 129;
	read_reg.rax.h = static_cast<decltype(read_reg.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_reg.rdi.r = reinterpret_cast<decltype(read_reg.rdi.r)>(&read_packet);

	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_reg);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);

	for (int i = 0; i < sizeof(s); i++) {
		fat_table_content.push_back(buffer[i]);
	}

	free(s);
	return fat_table_content;
}

void write_to_fs(int start, std::vector<char> data) {
	
	kiv_hal::TRegisters write_reg;
	kiv_hal::TDisk_Address_Packet write_packet;

	write_packet.count = data.size() / 512 + (data.size() % 512 != 0);
	write_packet.lba_index = static_cast<size_t>(start) + 31;
	
	write_reg.rdx.l = 129;
	write_reg.rax.h = static_cast<decltype(write_reg.rax.h)>(kiv_hal::NDisk_IO::Write_Sectors);
	write_reg.rdi.r = reinterpret_cast<decltype(write_reg.rdi.r)>(&write_packet);
	
	int last_sector_alloc = (start + static_cast<int>(write_packet.count)) - 1;
	std::vector<unsigned char> last_sector_data = read_from_fs(last_sector_alloc, 1);
	int size_to_hold = static_cast<int>(write_packet.count) * 512;

	int last_cluster_occupied = data.size() % 512;
	size_t start_pos = data.size();

	int added_bytes = 0;
	for (size_t i = start_pos; i < size_to_hold; i++) {
		data.push_back(last_sector_data.at(static_cast<size_t>(last_cluster_occupied) + static_cast<size_t>(added_bytes)));
		added_bytes++;
	}

	write_packet.sectors = static_cast<void*>(data.data());
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, write_reg);
}

std::vector<unsigned char> read_from_fs(int start, int sectors_count) {
	kiv_hal::TRegisters read_regs;
	kiv_hal::TDisk_Address_Packet read_packet;
	
	read_packet.count = sectors_count;
	auto s = malloc(512 * static_cast<size_t>(sectors_count));
	read_packet.sectors = (void*)s;
	read_packet.lba_index = static_cast<size_t>(start) + 31;
	read_regs.rdx.l = 129;
	read_regs.rax.h = static_cast<decltype(read_regs.rax.h)>(kiv_hal::NDisk_IO::Read_Sectors);
	read_regs.rdi.r = reinterpret_cast<decltype(read_regs.rdi.r)>(&read_packet);
	
	kiv_hal::Call_Interrupt_Handler(kiv_hal::NInterrupt::Disk_IO, read_regs);

	char* buffer = reinterpret_cast<char*>(read_packet.sectors);
	std::vector<unsigned char> content(buffer, buffer + (static_cast<size_t>(sectors_count) * 512));
	free(s);
	return content;
}

int create_folder(const char* name, uint8_t attr, std::vector<unsigned char>& fat_table, std::vector<int>& int_fat_table) {
	std::vector<std::string> folders_in_path = get_directories(name);
	
	std::string new_folder_name = folders_in_path.at(folders_in_path.size() - 1); 
	folders_in_path.pop_back(); 
	int start_sector = -1;
	std::vector<int> sectors_nums_data;
	if (folders_in_path.size() == 0) {
		printf(" root ");
		start_sector = 19;
		for (int i = 19; i < 33; i++) {
			sectors_nums_data.push_back(i);
		}
	}
	else {
		directory_item target_folder = retrieve_item(19, int_fat_table, folders_in_path);
		sectors_nums_data = retrieve_sectors_fs(int_fat_table, target_folder.first_cluster);

		start_sector = sectors_nums_data.at(0);
	}
	std::vector<directory_item> items_folder = retrieve_folders_from_folder(int_fat_table, start_sector); 

	int free_index = retrieve_free_index(int_fat_table);
	printf(" free: %d ", free_index);
	if (free_index == -1) { 
		return -1; 
	}
	else {
		std::vector<unsigned char> modified_bytes = num_to_bytes_fs(free_index, fat_table, 4095);
		fat_table.at(static_cast<int>(static_cast<double>(free_index) * 1.5)) = modified_bytes.at(0); 
		fat_table.at((static_cast<size_t>(static_cast<double>(free_index) * 1.5)) + 1) = modified_bytes.at(1);
		int_fat_table.at(free_index) = 4095;
		save_fat(fat_table); 
	}
	
	std::vector<unsigned char> to_write_subfolder;
	std::vector<char> to_save;
	//priprava entry
	int j = 0;
	for (; j < new_folder_name.length(); j++) {
		to_write_subfolder.push_back(new_folder_name.at(j));
	}
	for (; j < 8; j++) {
		to_write_subfolder.push_back(32);
	}
	for (int i = 0; i < 3; i++) { 
		to_write_subfolder.push_back(32);
	}
	to_write_subfolder.push_back(attr);
	for (int i = 0; i < 14; i++) { 
		to_write_subfolder.push_back(32);
	}
	std::vector<unsigned char> first_assigned_clust = dec_to_hex(free_index);
	for (int i = 0; i < 2; i++) {
		to_write_subfolder.push_back(first_assigned_clust.at(i));
	}
	for (int i = 0; i < 4; i++) {
		to_write_subfolder.push_back(0);
	}
	//konec pripravy
	if (folders_in_path.size() == 0) { 
		if ((items_folder.size() + 1 + 1) <= (sectors_nums_data.size() * 16)) { 
			size_t cluster_num = (items_folder.size() + 1) / 16; 
			size_t item_num_clust_rel = (items_folder.size() + 1) % 16;

			std::vector<unsigned char> data_clust = read_from_fs(sectors_nums_data.at(cluster_num) - 31, 1);

			for (int i = 0; i < to_write_subfolder.size(); i++) {
				data_clust.at((item_num_clust_rel * 32) + i) = to_write_subfolder.at(i);
			}

			for (int i = 0; i < data_clust.size(); i++) {
				to_save.push_back(data_clust.at(i));
			}
			write_to_fs(sectors_nums_data.at(cluster_num) - 31, to_save);
		}
		else {
			std::vector<unsigned char> modified_bytes = num_to_bytes_fs(free_index, fat_table, 0);
			fat_table.at(static_cast<int>(static_cast<double>(free_index) * 1.5)) = modified_bytes.at(0); 
			fat_table.at((static_cast<size_t>(static_cast<double>(free_index) * 1.5)) + 1) = modified_bytes.at(1);
			int_fat_table.at(free_index) = 0; 
			save_fat(fat_table);
			return -1;
		}
	}
	else {
		bool can_write = false; 
		if ((items_folder.size() + 2 + 1) <= (sectors_nums_data.size() * 16)) {
			can_write = true;
		}
		else { 
			int aloc_res = aloc_cluster(start_sector, int_fat_table, fat_table); 
			if (aloc_res == -1) {
				can_write = false;
			}
			else {
				can_write = true;
				sectors_nums_data.push_back(aloc_res);
			}
		}

		if (can_write) { 
			size_t cluster_num = (items_folder.size() + 2) / 16; 
			size_t item_num_clust_rel = (items_folder.size() + 2) % 16;

			std::vector<unsigned char> data_clust = read_from_fs(sectors_nums_data.at(cluster_num), 1);

			for (int i = 0; i < to_write_subfolder.size(); i++) {
				data_clust.at((item_num_clust_rel * 32) + i) = to_write_subfolder.at(i);
			}

			for (int i = 0; i < data_clust.size(); i++) {
				to_save.push_back(data_clust.at(i));
			}

			write_to_fs(sectors_nums_data.at(cluster_num), to_save);
		}
		else {
			std::vector<unsigned char> modified_bytes = num_to_bytes_fs(free_index, fat_table, 0);
			fat_table.at(static_cast<int>(static_cast<double>(free_index) * 1.5)) = modified_bytes.at(0);
			fat_table.at((static_cast<size_t>(static_cast<double>(free_index) * 1.5)) + 1) = modified_bytes.at(1);
			int_fat_table.at(free_index) = 0;

			save_fat(fat_table);

			return -1;
		}
	}
	write_folder_into_fs(free_index, start_sector);
	return 0;
}

void write_folder_into_fs(int index, int upper_index) {
	std::vector<char> subfolder;
	subfolder.push_back(46);
	for (int i = 0; i < 10; i++) {
		subfolder.push_back(32);
	}
	subfolder.push_back(0x10);
	for (int i = 0; i < 14; i++) {
		subfolder.push_back(32);
	}
	std::vector<unsigned char> first_assigned_clust = dec_to_hex(index);
	for (int i = 0; i < 2; i++) {
		subfolder.push_back(first_assigned_clust.at(i));
	}
	for (int i = 0; i < 4; i++) { 
		subfolder.push_back(0);
	}
	subfolder.push_back(46);
	subfolder.push_back(46);

	for (int i = 0; i < 9; i++) {
		subfolder.push_back(32);
	}
	subfolder.push_back(10);
	for (int i = 0; i < 14; i++) {
		subfolder.push_back(32);
	}
	if (upper_index == 19) { 
		upper_index = 0;
	}
	std::vector<unsigned char> upper_clust = dec_to_hex(upper_index);
	for (int i = 0; i < 2; i++) {
		subfolder.push_back(upper_clust.at(i));
	}
	for (int i = 0; i < 4; i++) {
		subfolder.push_back(0);
	}
	for (size_t i = subfolder.size(); i < 512; i++) {
		subfolder.push_back(0);
	}

	write_to_fs(index, subfolder);
}

std::vector<unsigned char> dec_to_hex(int start) {
	std::vector<unsigned char> converted;
	char convert_buffer[5]; 
	snprintf(convert_buffer, sizeof(convert_buffer), "%04X", start);

	char byte_to_save_first[3];
	char byte_to_save_second[3];
	unsigned char first_byte;
	unsigned char second_byte;

	byte_to_save_first[0] = convert_buffer[0];
	byte_to_save_first[1] = convert_buffer[1];
	byte_to_save_first[2] = '\0';

	byte_to_save_second[0] = convert_buffer[2];
	byte_to_save_second[1] = convert_buffer[3];
	byte_to_save_second[2] = '\0';

	first_byte = char_to_hex(byte_to_save_first);
	second_byte = char_to_hex(byte_to_save_second);

	converted.push_back(second_byte);
	converted.push_back(first_byte);

	return converted;

}

int retrieve_free_index(std::vector<int> int_fat_table) {
	for (int i = 0; i < 2848; i++) {
		if (int_fat_table.at(i) == 0) {
			return i;
		}
	}
	return -1;
}

std::vector<unsigned char> num_to_bytes_fs(int target, std::vector<unsigned char> fat_table, int num) {
	std::vector<unsigned char> converted_bytes;
	int first_index_hex_tab = static_cast<int>(static_cast<double>(target) * 1.5);
	unsigned char free_cluster_index_first = fat_table.at(first_index_hex_tab);
	unsigned char free_cluster_index_sec = fat_table.at(static_cast<size_t>(first_index_hex_tab) + 1);

	char convert_buffer[5];
	snprintf(convert_buffer, sizeof(convert_buffer), "%.2X%.2X", free_cluster_index_first, free_cluster_index_sec);
	char hex_free_clust[4];
	snprintf(hex_free_clust, sizeof(hex_free_clust), "%.3X", num);

	char byte_to_save_first[3];
	char byte_to_save_second[3];
	unsigned char first_byte;
	unsigned char second_byte;

	if (target % 2 == 0) {
		byte_to_save_first[0] = hex_free_clust[1]; 
		byte_to_save_first[1] = hex_free_clust[2]; 
		byte_to_save_second[0] = convert_buffer[2]; 
		byte_to_save_second[1] = hex_free_clust[0]; 
	}
	else { 
		byte_to_save_first[0] = hex_free_clust[2]; 
		byte_to_save_first[1] = convert_buffer[1]; 
		byte_to_save_second[0] = hex_free_clust[0]; 
		byte_to_save_second[1] = hex_free_clust[1]; 
	}

	byte_to_save_first[2] = '\0';
	byte_to_save_second[2] = '\0';
	first_byte = char_to_hex(byte_to_save_first);
	second_byte = char_to_hex(byte_to_save_second);

	converted_bytes.push_back(first_byte);
	converted_bytes.push_back(second_byte);

	return converted_bytes;

}

unsigned char char_to_hex(char charar[2]) {
	char ret[1];
	ret[0] = static_cast<char>(strtol(charar, NULL, 16));

	return ret[0];
}

std::vector<int> convert_fat_to_dec(std::vector<unsigned char> fat_table) {
	char firstbuffer[4];
	char secondbuffer[4];
	std::vector<int> fat_table_dec;
	char temp_buffer[7]; 
	int actual_number;

	for (int i = 0; i < fat_table.size();) {
		snprintf(temp_buffer, sizeof(temp_buffer), "%.2X%.2X%.2X", fat_table[i++], fat_table[i++], fat_table[i++]); //obsahuje 24 bitu
		snprintf(firstbuffer, sizeof(firstbuffer), "%c%c%c", temp_buffer[0], temp_buffer[1], temp_buffer[2]);
		snprintf(secondbuffer, sizeof(secondbuffer), "%c%c%c", temp_buffer[3], temp_buffer[4], temp_buffer[5]);

		actual_number = (int)strtol(secondbuffer, NULL, 16);
		fat_table_dec.push_back(actual_number);
		actual_number = (int)strtol(firstbuffer, NULL, 16);
		fat_table_dec.push_back(actual_number);
	}

	return fat_table_dec;
}

directory_item retrieve_item(int start_cluster, std::vector<int> int_fat_table, std::vector<std::string> path) {
	int traversed_sector_folder = start_cluster;
	int dir_item_number = -1;
	std::vector<directory_item> cur_folder_items;
	if (path.size() == 0) {
		directory_item dir_item;
		dir_item.filename = "\\";
		dir_item.extension = "";
		dir_item.filesize = 0;
		dir_item.first_cluster = 19;
		dir_item.attribute = static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID);

		return dir_item;
	}

	for (int i = 0; i < path.size(); i++) {
		dir_item_number = -1;
		cur_folder_items = retrieve_folders_from_folder(int_fat_table, traversed_sector_folder); 
		/*
		printf("\n");
		for (int i = 0; i < cur_folder_items.size(); i++) {
			printf("\njmeno slozky: %s ", cur_folder_items[i].filename.c_str());
			printf("attr: %c ", cur_folder_items[i].attribute);
			printf("first cluster: %d ", cur_folder_items[i].first_cluster);
			printf("size: %d", (int)cur_folder_items[i].filesize);
		}
		printf("\n");
		//*/
		int j = 0;

		std::string item_to_check = "";  
		while (dir_item_number == -1 && j < cur_folder_items.size()) { 
			directory_item dir_item = cur_folder_items.at(j);
			if (!dir_item.extension.empty()) {
				item_to_check = dir_item.filename + "." + dir_item.extension;
			}
			else {
				item_to_check = dir_item.filename;
			}
			if (path.at(i).compare(item_to_check) == 0) {
				dir_item_number = j;
			}

			j++;
		}

		if (dir_item_number != -1) { 
			directory_item dir_item = cur_folder_items.at(dir_item_number);
			traversed_sector_folder = dir_item.first_cluster;
			if (dir_item.first_cluster == 0) {
				dir_item.first_cluster = 19;
			}
		}
		else {
			break;
		}
	}
	directory_item dir_item{};
	if (dir_item_number != -1) { 
		dir_item = cur_folder_items.at(dir_item_number);
		if (dir_item.first_cluster == 0) { 
			dir_item.first_cluster = 19;
		}
	}
	else { 
		dir_item.first_cluster = -1;
		dir_item.filesize = -1;
	}
	return dir_item;
}

std::vector<kiv_os::TDir_Entry> get_os_dir_content(size_t sectors, std::vector<unsigned char> clusters, bool is_root) {
	std::vector<kiv_os::TDir_Entry> directory_content; 

	for (int i = 0; i < 512 * sectors;) {
		if (clusters[i] == 0 || clusters[i] == 246) {
			break;
		}

		kiv_os::TDir_Entry dir_item; 

		int j = 0;
		bool end_encountered = false;
		while (j < 8 && !end_encountered) { 
			if ((int)clusters[i] == 32) { 
				end_encountered = true;
				i += 8 - j; 
			}
			else {
				dir_item.file_name[j] = clusters[i++];
				j++;
			}
		}

		int position_filename = j; 

		dir_item.file_name[position_filename] = '.';
		position_filename++;

		j = 0;
		end_encountered = false;

		while (j < 3 && !end_encountered) { 
			if ((int)clusters[i] == 32) { 
				end_encountered = true;
				i += 3 - j; 
			}
			else {
				dir_item.file_name[position_filename] = clusters[i++];
				position_filename++;
				j++;
			}
		}

		if (dir_item.file_name[position_filename - 1] == '.') { 
			dir_item.file_name[position_filename - 1] = '\0';
		}
		else if (position_filename != 12) { 
			dir_item.file_name[position_filename] = '\0';
		}

		dir_item.file_attributes = clusters[i++];

		i += 14; 
		i += 6;

		directory_content.push_back(dir_item);
	}

	if (is_root) { 
		directory_content.erase(directory_content.begin()); 
	}
	else { 
		directory_content.erase(directory_content.begin()); 
		directory_content.erase(directory_content.begin());
	}

	return directory_content;

}

std::vector<directory_item> retrieve_folders_from_folder(std::vector<int> int_fat_table, int working_dir_sector) {
	if (working_dir_sector == 19) { 
		std::vector<unsigned char> root_dir = read_from_fs(19 - 31, 14);
		std::vector<directory_item> directory_content = get_dir_items(14, root_dir);
		directory_content.erase(directory_content.begin()); 
		return directory_content;
	}
	else { 
		std::vector<int> sectors_nums_data = retrieve_sectors_fs(int_fat_table, working_dir_sector); 
		std::vector<unsigned char> retrieved_data_clust;
		std::vector<directory_item> all_dir_items; 

		for (int i = 0; i < sectors_nums_data.size(); i++) { 
			retrieved_data_clust = read_from_fs(sectors_nums_data[i], 1); 
			std::vector<directory_item> directory_content = get_dir_items(1, retrieved_data_clust); 
			int j = 0;
			if (i == 0) { 
				j = 2;
			}
			else {
				j = 0;
			}

			for (; j < directory_content.size(); j++) { 
				directory_item dir_item = directory_content.at(j);

				all_dir_items.push_back(dir_item);
			}
		}

		return all_dir_items;
	}

}

std::vector<directory_item> get_dir_items(int sectornums, std::vector<unsigned char> dir_content) {
	std::vector<directory_item> directory_content;
	char first_clust_buff_conv[5]; 
	char filesize_buff_conv[9]; 

	for (int i = 0; i < 512 * sectornums;) {
		if (dir_content[i] == 0 || dir_content[i] == 246) {
			break;
		}

		directory_item dir_item;

		dir_item.filename = "";

		int j = 0;
		bool end_encountered = false; 
		while (j < 8 && !end_encountered) {
			if ((int)dir_content[i] == 32) { 
				end_encountered = true;
				i += 8 - j;
			}
			else {
				dir_item.filename.push_back(dir_content[i++]);
				j++;
			}
		}

		j = 0;
		end_encountered = false;
		dir_item.extension = "";
		while (j < 3 && !end_encountered) { 
			if ((int)dir_content[i] == 32) { 
				end_encountered = true;
				i += 3 - j; 
			}
			else {
				dir_item.extension.push_back(dir_content[i++]);
				j++;
			}
		}

		dir_item.attribute = dir_content[i++]; 
		i += 14; 

		snprintf(first_clust_buff_conv, sizeof(first_clust_buff_conv), "%.2X%.2X", dir_content[i++], dir_content[i++]);
		dir_item.first_cluster = (int)strtol(first_clust_buff_conv, NULL, 16);
		snprintf(filesize_buff_conv, sizeof(filesize_buff_conv), "%.2X%.2X%.2X%.2X", dir_content[i++], dir_content[i++], dir_content[i++], dir_content[i++]);
		dir_item.filesize = (int)strtol(filesize_buff_conv, NULL, 16);

		directory_content.push_back(dir_item);
	}

	return directory_content;
}

std::vector<int> retrieve_sectors_fs(std::vector<int> int_fat_table, int starting_sector) {
	std::vector<int> sector_list;
	int cluster_num = -1;
	int traversed_sector = starting_sector;
	while (traversed_sector < 4088 || traversed_sector > 4095) {
		if (sector_list.size() == 0) { 
			sector_list.push_back(traversed_sector);
			traversed_sector = int_fat_table[traversed_sector]; 
			continue;
		}

		sector_list.push_back(traversed_sector);
		traversed_sector = int_fat_table[traversed_sector];
	}

	return sector_list;
}

void save_fat(std::vector<unsigned char> fat_table) {
	std::vector<char> fat_table_to_save;
	for (int i = 0; i < fat_table.size(); i++) {
		fat_table_to_save.push_back(fat_table.at(i));
	}

	write_to_fs(1 - 31, fat_table_to_save); 
	write_to_fs(10 - 31, fat_table_to_save); 
}

void clear_fat_tables(std::vector<unsigned char> fat_table) {
	std::vector<char> clear_fat_table;
	for (int i = 0; i < fat_table.size(); i++) {
		clear_fat_table.push_back(30);
	}
	write_to_fs(1 - 31, clear_fat_table);
	write_to_fs(10 - 31, clear_fat_table);
}

int aloc_cluster(int start, std::vector<int>& int_fat_table, std::vector<unsigned char>& fat_table) {
	int free_index = retrieve_free_index(int_fat_table); 
	if (free_index == -1) {
		return -1;
	}
	else {
		std::vector<int> item_clusters = retrieve_sectors_fs(int_fat_table, start);
		std::vector<unsigned char> modified_bytes = num_to_bytes_fs(free_index, fat_table, 4095);
		fat_table.at(static_cast<int>(static_cast<double>(free_index) * 1.5)) = modified_bytes.at(0);
		fat_table.at((static_cast<size_t>(static_cast<double>(free_index) * 1.5)) + static_cast <int>(1)) = modified_bytes.at(1);
		int_fat_table.at(free_index) = 4095;
		modified_bytes = num_to_bytes_fs(item_clusters.at(item_clusters.size() - 1), fat_table, free_index);
		fat_table.at(static_cast<int>(static_cast<double>(item_clusters.at(item_clusters.size() - 1)) * 1.5)) = modified_bytes.at(0);
		fat_table.at((static_cast<size_t>(static_cast<double>(item_clusters.at(item_clusters.size() - 1)) * 1.5)) + static_cast <int>(1)) = modified_bytes.at(1);
		int_fat_table.at(item_clusters.at(item_clusters.size() - 1)) = free_index;
		save_fat(fat_table);
		return free_index;
	}

}