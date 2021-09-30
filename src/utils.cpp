#include "breakout/utils.h"
#include "breakout/log.h"

#include <fstream>
#include <sstream>

std::string ReadFile(const char* filepath) {
	std::string text;
	if (TryReadFile(filepath, text))
		return text;
	else
		throw std::exception();
}

bool TryReadFile(const char* filepath, std::string& out_text) {
	using namespace std;

	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try {
		std::stringstream ss;

		file.open(filepath);
		ss << file.rdbuf();
		file.close();

		out_text = ss.str();
	}
	catch (std::ifstream::failure&) {
		LOG(LOG_DEBUG, "ReadFile - Failed to read from '%s'.\n", filepath);
		return false;
	}

	LOG(LOG_RESOURCE, "ReadFile - Loaded file '%s' (%d)\n", filepath, (int)out_text.size());
	return true;
}
