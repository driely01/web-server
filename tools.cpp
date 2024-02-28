#include "Server.hpp"

size_t get_size_fd(int fd) {
	struct stat fileStat;

	if (fstat(fd, &fileStat) == -1) {
		std::cerr << "Failed to get file status." << std::endl;
		close(fd);
		exit( EXIT_FAILURE );
	}
	return (fileStat.st_size);
}