#pragma once

#include <optional>
#include <vector>
#include <cstddef>
#include <filesystem>

std::optional<std::vector<std::byte>> LoadFileBinary(std::filesystem::path filePath);