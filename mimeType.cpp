#include "Server.hpp"

void Server::setMimeType(std::string& mimetype) {
	this->mimetypes = ".aac|audio/aac\n\\
.abw|application/x-abiword\n\\
.apng|image/apng\n\\
.arc|application/x-freearc\n\\
.avif|image/avif\n\\
.avi|video/x-msvideo\n\\
.azw|application/vnd.amazon.ebook\n\\
.bin|application/octet-stream\n\\
.bmp|image/bmp\n\\
.bz|application/x-bzip\n\\";
}