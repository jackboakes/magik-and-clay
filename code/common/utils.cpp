#include "utils.h"

#include <fstream>
#include <utility>

std::optional<std::vector<std::byte>> LoadFileBinary(std::filesystem::path filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    std::optional<std::vector<std::byte>> result;

    if (file.good())
    {
        auto fileSize { std::filesystem::file_size(filePath) };
        std::vector<std::byte> data(fileSize);
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        file.close();
        result = std::move(data);
    }

    // TODO:: logging

    return result;
}