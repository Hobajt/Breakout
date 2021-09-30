#pragma once

#include <string>

std::string ReadFile(const char* filepath);
bool TryReadFile(const char* filepath, std::string& out_text);
