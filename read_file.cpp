#include <iostream>
#include <fstream>

int main()
{
    size_t N = 10000; 
    std::ifstream file("state.s", std::ios::binary | std::ios::in); 

    for (int p = 0; p < 10000 * sizeof(uint64_t); p += sizeof(uint64_t))
    {
        uint64_t value = {};
        file.seekg(p, std::ios::beg); 
        file.read(reinterpret_cast<char*>(&value), sizeof(uint64_t));
        std::cout << "#" << p / sizeof(uint64_t) << 
                     " " << value << std::endl;
    }

    return 0;
}
