#include "File.hpp"

File::File(std::string filename, std::string sender, std::string receiver, std::string path)
{
	_filename = filename;
	_sender = sender;
	_receiver = receiver;
	_path = path;
}

File::~File(){}

File &File::operator=(const File &src)
{
	_filename = src._filename;
	_sender = src._sender;
	_receiver = src._receiver;
	_path = src._path;
	return *this;
}

std::string const &File::getFilename() const {return _filename;}

std::string const &File::getSender() const {return _sender;}

std::string const &File::getReceiver() const {return _receiver;}

std::string const &File::getPath() const {return _path;}
