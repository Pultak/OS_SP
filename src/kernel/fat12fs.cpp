#include "fat12fs.h"
#include <iostream>

std::vector<unsigned char> fat_table;

std::vector<int> int_fat_table;

FAT::FAT(uint8_t disk_num, kiv_hal::TDrive_Parameters params) : disk(disk_num), parameters(params) {
    fat_table = load_first_table(); // nacteni fat tabulky
    int_fat_table = convert_fat_to_dec(fat_table); // prevedeni fat tabulky do podoby int cisel
}

kiv_os::NOS_Error FAT::rmdir(const char* pth) {
    std::vector<std::string> path = get_directories(pth); // nalezeni cesty
    directory_item dir_item = retrieve_item(19, int_fat_table, path); // nacteni directory_itemu na zadane ceste

    if (dir_item.first_cluster == -1) { // directory_item nebyl nalezen
        return kiv_os::NOS_Error::File_Not_Found;
    }
    

    std::vector<int> sectors_nums_data = retrieve_sectors_fs(int_fat_table, dir_item.first_cluster); // nalezeni sektoru pripadajicich directory_itemu
    std::vector<directory_item> items_folder = retrieve_folders_from_folder(int_fat_table, dir_item.first_cluster); // nacteni slozek ve slozce
    if (items_folder.size() != 0) { 
        return kiv_os::NOS_Error::Directory_Not_Empty;
    }

    // zapsani prazdnych znaku na misto slozky a uvolneni sektoru ve fat tabulce
    std::vector<char> zero_content;
    for (int i = 0; i < SECTOR_SIZE; i++) { 
        zero_content.push_back(0);
    }
    for (int i = 0; i < sectors_nums_data.size(); i++) {
        std::vector<unsigned char> modified_bytes = num_to_bytes_fs(sectors_nums_data.at(i), fat_table, 0);
        fat_table.at(static_cast<size_t>(static_cast<double>(sectors_nums_data.at(i)) * 1.5)) = modified_bytes.at(0); 
        fat_table.at(static_cast<size_t>(static_cast<double>(sectors_nums_data.at(i) * 1.5)) + 1) = modified_bytes.at(1);
        int_fat_table.at(sectors_nums_data.at(i)) = 0;
        write_to_fs(sectors_nums_data.at(i), zero_content);
    }

    save_fat(fat_table);

    //mazani slozky z danrazene slozky
    std::string filename = path.at(path.size() - 1);
    path.pop_back();
    bool upper_root = false;
    int start_sector = -1;
    std::vector<int> data_upper;

    if (path.size() == 0) { 
        upper_root = true;
        start_sector = 19;
        for (int i = 19; i < 33; i++) {
            data_upper.push_back(i);
        }
    }
    else { 
        directory_item target_folder = retrieve_item(19, int_fat_table, path);
        data_upper = retrieve_sectors_fs(int_fat_table, target_folder.first_cluster);
        start_sector = data_upper.at(0);
    }

    std::vector<directory_item> folders_upper = retrieve_folders_from_folder(int_fat_table, start_sector); // obsah nadrazene slozky
    int id_remove = -1;

    for (int i = 0; i < folders_upper.size(); i++) {
        std::string item_to_check = "";
        directory_item di = folders_upper.at(i);
        if (!di.extension.empty()) {
            item_to_check = di.filename + "." + di.extension;
        }
        else {
            item_to_check = di.filename;
        }

        if (item_to_check.compare(filename) == 0) { // nalezeniu slozky
            id_remove = i;
            break;
        }
    }
     // ziskani obsahu slozky
    std::vector<char> full_fol_cont;
    for (int i = 0; i < data_upper.size(); i++) {
        std::vector<unsigned char> one_clust;
        if (upper_root) {
            one_clust = read_from_fs(data_upper.at(i) - 31, 1);
        }
        else {
            one_clust = read_from_fs(data_upper.at(i), 1);
        }

        for (int j = 0; j < SECTOR_SIZE; j++) {
            full_fol_cont.push_back(one_clust.at(j));
        }
    }

    int index_to_remove;
    if (upper_root) {
        index_to_remove = id_remove + 1;
    }
    else {
        index_to_remove = id_remove + 2;
    }

    full_fol_cont.erase(full_fol_cont.begin() + (static_cast<size_t>(index_to_remove) * 32), full_fol_cont.begin() + ((static_cast<size_t>(index_to_remove) * 32) + 32));

    for (int i = 0; i < 32; i++) {
        full_fol_cont.push_back(0);
    }

    for (int i = 0; i < data_upper.size(); i++) {
        if (upper_root) {
            write_to_fs(data_upper.at(i) - 31, zero_content);
        }
        else {
            write_to_fs(data_upper.at(i), zero_content);
        }
    }
 
    for (int i = 0; i < data_upper.size(); i++) {
        std::vector<char> clust_to_save;
        for (int j = 0; j < SECTOR_SIZE; j++) {
            clust_to_save.push_back(full_fol_cont.at((static_cast<size_t>(i) * SECTOR_SIZE) + j));
        }
        if (upper_root) {
            write_to_fs(data_upper.at(i) - 31, clust_to_save);
        }
        else {
            write_to_fs(data_upper.at(i), clust_to_save);
        }
    }
    // odkaz na slozku z nadrazene slozky vymazan
    return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FAT::mkdir(const char* pth, uint8_t attr) {
    std::vector<std::string> folders_in_path = get_directories(pth);

    std::string new_folder_name = folders_in_path.at(folders_in_path.size() - 1);
    folders_in_path.pop_back();
    int result = create_folder(pth, attr, fat_table, int_fat_table);
    // kontrola uspechu zapsani slozky
    if (result == 0) {
        return kiv_os::NOS_Error::Success;
    }
    else {
        return kiv_os::NOS_Error::Not_Enough_Disk_Space;
    }
}

kiv_os::NOS_Error FAT::open(const char* pth, kiv_os::NOpen_File flags, uint8_t attributes, File* file) {
    file->name = const_cast<char*>(pth);
    file->position = 0;
    int32_t target_cluster;

    std::vector<std::string> folders_in_path = get_directories(pth);
    if (folders_in_path.size() > 0 && strcmp(folders_in_path.at(folders_in_path.size() - 1).data(), ".") == 0) { // odstranit .
        folders_in_path.pop_back();
    }
    directory_item dir_item = retrieve_item(19, int_fat_table, folders_in_path); // otevreni souboru/slozky na ceste
    target_cluster = dir_item.first_cluster;
    if (target_cluster == -1) { // pokud soubor/slozka jeste neexistuje
        if (flags == kiv_os::NOpen_File::fmOpen_Always) {
            return kiv_os::NOS_Error::File_Not_Found;
        }
        else {
            dir_item.attribute = attributes;

            dir_item.filesize = 0;

            bool created = false;
            //jedna se o slozku
            if (dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID) || dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) {
                if (!folder_name_val(folders_in_path.at(folders_in_path.size() - 1).c_str())){
                    return kiv_os::NOS_Error::Invalid_Argument;
                }
                kiv_os::NOS_Error result = mkdir(pth, attributes);
                if (result == kiv_os::NOS_Error::Not_Enough_Disk_Space) {
                    created = false;
                }
                else { 
                    created = true;
                }
            }
            else { // jedna se o soubor
                if (!file_name_val(folders_in_path.at(folders_in_path.size() - 1).c_str())) { 
                    return kiv_os::NOS_Error::Invalid_Argument;
                }
                int result = create_file(pth, attributes, fat_table, int_fat_table);

                file->size = dir_item.filesize;

                if (result == -1) { 
                    created = false;
                }
                else { 
                    created = true;
                }
            }

            if (!created) { 
                return kiv_os::NOS_Error::Not_Enough_Disk_Space;
            }
            else { 
                target_cluster = retrieve_item(19, int_fat_table, folders_in_path).first_cluster;
            }
        }
    }
    else if ((dir_item.attribute & static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) && ((attributes & static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) == 0)) {
        return kiv_os::NOS_Error::Permission_Denied;
    }
    // zde jiz soubor/slozka existuje, nebo byla vytvorena
    file->attributes = dir_item.attribute;
    file->handle = target_cluster;
    std::vector<int> sector_nums = retrieve_sectors_fs(int_fat_table, file->handle);
    if (dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID) || dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) {
        std::vector<kiv_os::TDir_Entry> dir_entries_size; 
        dirread(file->name, dir_entries_size);
        dir_item.filesize = dir_entries_size.size() * sizeof(kiv_os::TDir_Entry);
        file->size = dir_item.filesize;
    }
    else {
        file->size = dir_item.filesize;
    }
    return kiv_os::NOS_Error::Success;
    
}

kiv_os::NOS_Error FAT::read(File* f, size_t size, size_t offset, std::vector<char>& out) {
    //cteni slozky
    if (((f->attributes & static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID)) != 0) || ((f->attributes & static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) != 0)) {
        //offset = static_cast<uint8_t>(0);
        std::vector<kiv_os::TDir_Entry> folders;
        std::vector<char> folders_char;
        kiv_os::NOS_Error read_dir_res = dirread(f->name, folders); // ziskani slozek ve formatu TDir_Entry
        if (read_dir_res == kiv_os::NOS_Error::Success) { // prevedeni na chary
            folders_char = convert_dirs_to_chars(folders); 
        }
        if ((offset + size) <= (folders_char.size() * sizeof(kiv_os::TDir_Entry))) { 
            for (int i = 0; i < size; i++) {
                out.push_back(folders_char.at(offset + i));
            }
            return kiv_os::NOS_Error::Success;
        }
        else { 
            return kiv_os::NOS_Error::IO_Error;
        }
    }
    else {
        //offset = static_cast<uint8_t>(0);
        if ((offset + size) > f->size) {
            return kiv_os::NOS_Error::IO_Error;
        }
        std::vector<int> file_clust_nums = retrieve_sectors_fs(int_fat_table, f->handle); // ziskani sektoru pripadajicich souboru
        std::vector<unsigned char> clust_content;
        size_t sector;

        if (offset == 0) {
            sector = 1;
        }
        else {
            sector = (offset / SECTOR_SIZE) + 1;
        }
        int sector_num_vect = file_clust_nums.at(sector - 1);
        int skip = offset % SECTOR_SIZE;
        clust_content = read_from_fs(sector_num_vect, 1);
        for (int i = skip; i < SECTOR_SIZE; i++) {
            if (out.size() == size) {
                break;
            }
            else {
                out.push_back(clust_content.at(i));
            }
        }
        sector++;
        while (out.size() != size) {
            clust_content = read_from_fs(file_clust_nums.at(sector - 1), 1);
            sector++;

            for (int i = 0; i < SECTOR_SIZE; i++) {
                if (out.size() == size) {
                    break;
                }
                else {
                    out.push_back(clust_content.at(i));
                }
            }
        }
        return kiv_os::NOS_Error::Success;
    }
}

kiv_os::NOS_Error FAT::dirread(const char* pth, std::vector<kiv_os::TDir_Entry>& entries) {
    std::vector<std::string> folders_in_path = get_directories(pth);
    if (folders_in_path.size() > 0 && strcmp(folders_in_path.at(folders_in_path.size() - 1).data(), ".") == 0) {
        folders_in_path.pop_back();
    }

    if (folders_in_path.size() == 0) { // root
        std::vector<unsigned char> root_dir_cont = read_from_fs(19 - 31, 14);
        entries = get_os_dir_content(14, root_dir_cont, true);

        return kiv_os::NOS_Error::Success;
    }
    directory_item dir_item = retrieve_item(19, int_fat_table, folders_in_path); // nalezeni slozky
    int first_cluster_fol = dir_item.first_cluster;
    std::vector<int> folder_cluster_nums = retrieve_sectors_fs(int_fat_table, first_cluster_fol);
    std::vector<unsigned char> one_clust_data;
    std::vector<unsigned char> all_clust_data;
    for (int i = 0; i < folder_cluster_nums.size(); i++) {
        one_clust_data = read_from_fs(folder_cluster_nums[i], 1); 
        for (int j = 0; j < one_clust_data.size(); j++) { 
            all_clust_data.push_back(one_clust_data.at(j));
        }
    }
    entries = get_os_dir_content(folder_cluster_nums.size(), all_clust_data, false);
    return kiv_os::NOS_Error::Success;

}

std::vector<char> FAT::convert_dirs_to_chars(const std::vector<kiv_os::TDir_Entry>& directories) {
    std::vector<char> res;
    for (auto dir : directories) {
        auto const ch_dir = reinterpret_cast<char*>(&dir);
        res.insert(res.end(), ch_dir, ch_dir + sizeof dir);
    }
    return res;
}

bool FAT::file_exist(const char* pth) {
    int start_cluster = 19;
    std::vector<std::string> folders_in_path = get_directories(pth);
    directory_item dir_item = retrieve_item(start_cluster, int_fat_table, folders_in_path);
    if (dir_item.first_cluster == -1) {
        return false;
    }
    else {
        return true;
    }
}

kiv_os::NOS_Error FAT::write(File* f, size_t size, size_t offset, std::vector<char> buf, size_t& written) {
    if (offset > f->size) {
        return kiv_os::NOS_Error::IO_Error;
    }
    std::vector<int> file_nums = retrieve_sectors_fs(int_fat_table, f->handle); // clustery na kterych se soubor nachazi
    size_t sector;
    if (offset == 0) {
        sector = 1;
    }
    else {
        sector = (offset / SECTOR_SIZE) + 1;
    }

    if (sector > file_nums.size()) {
        int result = aloc_cluster(file_nums.at(0), int_fat_table, fat_table);
        if (result == -1) {
            return kiv_os::NOS_Error::Not_Enough_Disk_Space;
        }
        file_nums.push_back(result);
    }
    int sector_num = file_nums.at(sector - 1); //nalezeni odpovidajiciho sektoru
    int bytes_to_save_clust = offset % SECTOR_SIZE;

    std::vector<unsigned char> data_last_clust = read_from_fs(sector_num, 1);
    std::vector<unsigned char> data_to_write;

    for (int i = 0; i < bytes_to_save_clust; i++) {
        data_to_write.push_back(data_last_clust.at(i));
    }

    for (int i = 0; i < buf.size(); i++) {
        data_to_write.push_back(buf.at(i));
    }
    std::string content_clust;
    for (int i = 0; i < data_to_write.size(); i++) {
        content_clust.push_back(data_to_write.at(i));
    }

    size_t written_bytes = 0 - static_cast<size_t>(bytes_to_save_clust);
    //v bufferu jsou ulozena data ktera mame zapsat.
    size_t clusters_count = data_to_write.size() / SECTOR_SIZE + (data_to_write.size() % SECTOR_SIZE != 0);

    std::vector<char> clust_data_write;
    for (int i = 0; i < clusters_count; i++) {
        if (i == (clusters_count - 1)) {
            for (int j = 0; j < data_to_write.size() - (static_cast<size_t>(i) * SECTOR_SIZE); j++) {
                clust_data_write.push_back(data_to_write.at(j + (static_cast<size_t>(i) * SECTOR_SIZE)));
            }
        }
        else { 
            for (int j = 0; j < SECTOR_SIZE; j++) {
                clust_data_write.push_back(data_to_write.at(j + (static_cast<size_t>(i) * SECTOR_SIZE)));
            }
        }
        if (sector - 1 + i < file_nums.size()) {
            write_to_fs(file_nums.at(sector - 1 + i), clust_data_write);
            written_bytes += clust_data_write.size();
        }
        else {
            int free_clust_index = retrieve_free_index(int_fat_table);
            if (free_clust_index == -1) {
                save_fat(fat_table); 
                size_t written_size = (offset + written_bytes) - f->size;
                if (written_size > 0) {
             
                    update_file_size(f->name, offset, f->size, written_size, int_fat_table);
                    f->size = f->size + written_size;
                    written = written_bytes;
                }
                return kiv_os::NOS_Error::Not_Enough_Disk_Space; 
            }
           
            int index_to_edit = file_nums.at(file_nums.size() - 1); 
            int first_index_hex_tab = static_cast<int>(static_cast<double>(index_to_edit) * 1.5); 
            char free_cluster_index_first = fat_table.at(first_index_hex_tab);
            char free_cluster_index_sec = fat_table.at(static_cast<size_t>(first_index_hex_tab) + 1);
            int_fat_table.at(file_nums.at(file_nums.size() - 1)) = free_clust_index;
            int_fat_table.at(free_clust_index) = 4095;
            std::vector<unsigned char> modified_bytes = num_to_bytes_fs(index_to_edit, fat_table, free_clust_index);
            fat_table.at(static_cast<int>(static_cast<double>(index_to_edit) * 1.5)) = modified_bytes.at(0);
            fat_table.at((static_cast<size_t>(static_cast<double>(index_to_edit) * 1.5)) + 1) = modified_bytes.at(1);

            
            first_index_hex_tab = static_cast<int>(static_cast<double>(free_clust_index) * 1.5); 
            free_cluster_index_first = fat_table.at(first_index_hex_tab);
            free_cluster_index_sec = fat_table.at(static_cast<size_t>(first_index_hex_tab) + 1);
            modified_bytes = num_to_bytes_fs(free_clust_index, fat_table, 4095);
            fat_table.at(static_cast<int>(static_cast<double>(free_clust_index) * 1.5)) = modified_bytes.at(0);
            fat_table.at((static_cast<size_t>(static_cast<double>(free_clust_index) * 1.5)) + 1) = modified_bytes.at(1);
           
            write_to_fs(free_clust_index, clust_data_write); 
            written_bytes += clust_data_write.size();
        }
        clust_data_write.clear();
    }
    save_fat(fat_table);
    size_t written_size = (offset + written_bytes) - f->size;
    if (written_size > 0) {
        update_file_size(f->name, offset, f->size, written_size, int_fat_table);
        f->size = f->size + written_size;
    }
    written = written_bytes;
    return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FAT::set_file_attribute(const char* name, uint8_t attribute) {
    std::vector<std::string> folders = get_directories(name); 
    std::string filename = folders.at(folders.size() - 1);
    folders.pop_back();

    int start = -1;
    std::vector<int> sectors;
    if (folders.size() == 0) {
        start = 19;
        for (int i = 19; i < 33; i++) {
            sectors.push_back(i);
        }
    }
    else {
        directory_item target = retrieve_item(19, int_fat_table, folders);
        sectors = retrieve_sectors_fs(int_fat_table, target.first_cluster);
        start = sectors.at(0);
    }
    if (start == -1) {
        return kiv_os::NOS_Error::File_Not_Found;
    }

    int target_index = 0;
    std::vector<directory_item> item_folder = retrieve_folders_from_folder(int_fat_table, start);
    for (int i = 0; i < item_folder.size(); i++) {
        std::string item_to_check = "";
        directory_item dir_item = item_folder.at(i);

        if (!dir_item.extension.empty()) {
            item_to_check = dir_item.filename + "." + dir_item.extension;
        }
        else {
            item_to_check = dir_item.filename;
        }

        if (item_to_check.compare(filename) == 0) { 
            target_index = i;
            break;
        }
    }
    if (target_index == 0) {
        return kiv_os::NOS_Error::File_Not_Found;
    }
    if (folders.size() == 0) {
        target_index += 1;
    }
    else {
        target_index += 2;
    }

    int clust_num = (target_index) / 16; 
    int clust_rel = (target_index) % 16;
    std::vector<unsigned char> data_fol; 
    if (folders.size() == 0) {
        data_fol = read_from_fs(sectors.at(clust_num) - 31, 1);
        data_fol.at(static_cast<size_t>(clust_rel) * 32 + 11) = attribute;
        std::vector<char> data_to_save;
        for (int i = 0; i < data_fol.size(); i++) {
            data_to_save.push_back(data_fol.at(i));
        }
        write_to_fs(sectors.at(clust_num) - 31, data_to_save);
    }
    else {
        data_fol = read_from_fs(sectors.at(clust_num), 1);
        data_fol.at(static_cast<size_t>(clust_rel) * 32 + 11) = attribute;
        std::vector<char> data_to_save;
        for (int i = 0; i < data_fol.size(); i++) {
            data_to_save.push_back(data_fol.at(i));
        }
        write_to_fs(sectors.at(clust_num), data_to_save);
    }
    return kiv_os::NOS_Error::Success;
}

kiv_os::NOS_Error FAT::get_file_attribute(const char* name, uint8_t& out_attribute) {
    std::vector<std::string> folders = get_directories(name);
    std::string filename = folders.at(folders.size() - 1);
    folders.pop_back();

    int start = -1;
    std::vector<int> sectors_data;
    if (folders.size() == 0) {
        start = 19;
        for (int i = 19; i < 33; i++) {
            sectors_data.push_back(i);
        }
    }
    else {
        directory_item target_folder = retrieve_item(19, int_fat_table, folders);
        sectors_data = retrieve_sectors_fs(int_fat_table, target_folder.first_cluster);
        start = sectors_data.at(0);
    }
    if (start == -1) {
        return kiv_os::NOS_Error::File_Not_Found;
    }

    std::vector<directory_item> folder = retrieve_folders_from_folder(int_fat_table, start);

    int target_index = 0;
    for (int i = 0; i < folder.size(); i++) {
        std::string item_to_check = "";
        directory_item dir_item = folder.at(i);
        if (!dir_item.extension.empty()) {
            item_to_check = dir_item.filename + "." + dir_item.extension;
        }
        else {
            item_to_check = dir_item.filename;
        }
        if (item_to_check.compare(filename) == 0) {
            target_index = i;
            break;
        }
    }
    if (target_index == -1) {
        return kiv_os::NOS_Error::File_Not_Found;
    }
    if (folders.size() == 0) {
        target_index += 1;
    }
    else {
        target_index += 2;
    }

    int clust_num = (target_index) / 16;
    int clust_rel = (target_index) % 16;
    std::vector<unsigned char> data_fol;
    if (folders.size() == 0) {
        data_fol = read_from_fs(sectors_data.at(clust_num) - 31, 1);
        out_attribute = data_fol.at(static_cast<size_t>(clust_rel) * 32 + 11);
    }
    else {
        data_fol = read_from_fs(sectors_data.at(clust_num), 1);
        out_attribute = data_fol.at(static_cast<size_t>(clust_rel) * 32 + 11);
    }
    return kiv_os::NOS_Error::Success;
}