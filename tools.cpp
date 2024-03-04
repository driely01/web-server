#include "Server.hpp"

size_t get_size_fd(std::string fileD) {
	struct stat fileStat;

	if (lstat(fileD.c_str(), &fileStat) == -1) {
		return (-1);
	}
	return (fileStat.st_size);
}

size_t stringToInt(const std::string& str) {
	std::istringstream iss(str);
	size_t result;
	iss >> result;
	return result;
}

std::string intToString(size_t num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}