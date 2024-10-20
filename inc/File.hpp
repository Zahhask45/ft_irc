#ifndef FILE_HPP
#define FILE_HPP

#include "Server.hpp"
#include <sys/stat.h>

class File
{
private:
	std::string _filename;
	std::string _sender;
	std::string _receiver;
	std::string _path;
public:
	File(std::string filename, std::string sender, std::string receiver, std::string path);
	~File();

	File &operator=(const File &src);
	std::string const &getFilename() const;
	std::string const &getSender() const;
	std::string const &getReceiver() const;
	std::string const &getPath() const;
};

#endif
