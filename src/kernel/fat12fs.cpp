#include "fat12fs.h"

std::vector<unsigned char> fat_table;

std::vector<int> int_fat_table;

FAT::FAT(uint8_t disk_num, kiv_hal::TDrive_Parameters params): disk(disk_num), parameters(params) {
    //clear_fat_tables(fat_table);
    
	fat_table = load_first_table();
    int_fat_table = convert_fat_to_dec(fat_table);
    /*
    printf("%d", retrieve_free_index(int_fat_table));
    for (int i = 0; i < fat_table.size(); i++) {
        printf("hex: %c", fat_table.at(i));
    }
    
    for (int i = 0; i < int_fat_table.size(); i++) {
        printf("%d: %d\n", i, int_fat_table.at(i));
    }
    //*/
    
    
}

kiv_os::NOS_Error FAT::mkdir(const char* name, uint8_t attr){
    std::vector<std::string> folders_in_path = get_directories(name);
    
    std::string new_folder_name = folders_in_path.at(folders_in_path.size() - 1);
    folders_in_path.pop_back(); 
    int result = create_folder(name, attr, fat_table, int_fat_table);

    if (result == 0) {
        printf(" podarilo se vytvorit slozku ");
        return kiv_os::NOS_Error::Success;
    }
    else {
        printf(" nepodarilo se vytvorit slozku ");
        return kiv_os::NOS_Error::Not_Enough_Disk_Space;
    }
}

kiv_os::NOS_Error FAT::open(const char* name, kiv_os::NOpen_File flags, uint8_t attributes, File& file) {
    file = File{};
    file.name = const_cast<char*>(name);
    file.position = 0;

    int32_t target_cluster;
    
    std::vector<std::string> folders_in_path = get_directories(name);
    if (folders_in_path.size() > 0 && strcmp(folders_in_path.at(folders_in_path.size() - 1).data(), ".") == 0) {
        printf(" nalezena cesta > 0 ");
        folders_in_path.pop_back();
    }
    directory_item dir_item = retrieve_item(19, int_fat_table, folders_in_path);
    target_cluster = dir_item.first_cluster;
    if (target_cluster == -1) {
        if (flags == kiv_os::NOpen_File::fmOpen_Always) {
            return kiv_os::NOS_Error::File_Not_Found;
        }
        else { 
            dir_item.attribute = attributes;
            dir_item.filesize = 0;

            bool created = false;

            if (dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID) || dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) {
                kiv_os::NOS_Error result = mkdir(name, attributes);
                if (result == kiv_os::NOS_Error::Not_Enough_Disk_Space) {
                    created = false;
                }
                else { 
                    created = true;
                }
            }
            else {
                printf(" vytvareni neceho jineho ");
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
    file.attributes = dir_item.attribute;
    file.handle = target_cluster;
    std::vector<int> sector_nums = retrieve_sectors_fs(int_fat_table, file.handle);
    if (dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Volume_ID) || dir_item.attribute == static_cast<uint8_t>(kiv_os::NFile_Attributes::Directory)) {
        std::vector<kiv_os::TDir_Entry> dir_entries_size; 
        dirread(file.name, dir_entries_size);
        dir_item.filesize = dir_entries_size.size() * sizeof(kiv_os::TDir_Entry);
        file.size = dir_item.filesize;
    }
    else {
        file.size = dir_item.filesize; 
    }
    
    return kiv_os::NOS_Error::Success;
    
}

kiv_os::NOS_Error FAT::dirread(const char* name, std::vector<kiv_os::TDir_Entry>& entries) {
    std::vector<std::string> folders_in_path = get_directories(name);
    if (folders_in_path.size() > 0 && strcmp(folders_in_path.at(folders_in_path.size() - 1).data(), ".") == 0) {
        folders_in_path.pop_back();
    }

    if (folders_in_path.size() == 0) { 
        std::vector<unsigned char> root_dir_cont = read_from_fs(19 - 31, 14);
        entries = get_os_dir_content(14, root_dir_cont, true);

        return kiv_os::NOS_Error::Success;
    }
    directory_item dir_item = retrieve_item(19, int_fat_table, folders_in_path);
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

