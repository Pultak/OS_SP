#include <vector>
#include <string>

std::vector<std::string> get_directories(const char* path);

void write_to_fs(int start, std::vector<char> data);

std::vector<unsigned char> read_data(int start, int sectors_count);

std::vector<unsigned char> load_first_table();

std::vector<unsigned char> load_second_table();