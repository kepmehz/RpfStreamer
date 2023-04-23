#include "scanner.h"

scanner::scanner(const char* module_name)
{
    this->module_address = reinterpret_cast<uintptr_t>(GetModuleHandleA(module_name));
}

std::vector<int> scanner::ConvPatternToByte(const char* pattern)
{
    auto bytes = std::vector<int>();

    auto pattern_start = const_cast<char*>(pattern);
    auto pattern_end = const_cast<char*>(pattern) + strlen(pattern);

    for (auto current_byte = pattern_start; current_byte < pattern_end; ++current_byte)
    {
        if (*current_byte == '?') // if it's null
        {
            ++current_byte;
            if (*current_byte == '?') ++current_byte;
            bytes.push_back(-1);
        }
        else bytes.push_back(strtoul(current_byte, &current_byte, 16));
    }

    return bytes;
}

Handle scanner::scan(const char* pattern)
{
    auto dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(this->module_address);
    auto nt_header = reinterpret_cast<IMAGE_NT_HEADERS*>(reinterpret_cast<std::uint8_t*>(this->module_address) + dos_header->e_lfanew);

    auto size = nt_header->OptionalHeader.SizeOfImage;
    auto pattern_bytes = this->ConvPatternToByte(pattern);
    auto start_module = reinterpret_cast<std::uint8_t*>(this->module_address);

    for (auto i = 0; i < size - pattern_bytes.size(); ++i)
    {
        bool found_byte_set = true;
        for (auto j = 0; j < pattern_bytes.size(); ++j)
        {
            if (start_module[i + j] != pattern_bytes.data()[j] && pattern_bytes.data()[j] != -1)
            {
                found_byte_set = false;
                break;
            }
        }
        if (found_byte_set)
            return Handle(reinterpret_cast<std::uintptr_t>(&start_module[i]));
    }
    return Handle();
}