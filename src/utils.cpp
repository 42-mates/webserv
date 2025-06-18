#include <string>
#include <sys/stat.h>
#include "utils.hpp"

bool	is_directory(const std::string& path)
{
	struct stat	s;

	if (stat(path.c_str(), &s) == 0)
		if (s.st_mode & S_IFDIR)
			return true;
	return false;
}

bool	end_with_json(const std::string& s)
{
	std::string	suffix = ".json";

	if (s.length() >= suffix.length())
		return (s.compare(s.length() - suffix.length(), suffix.length(), suffix) == 0);
	return false;
	}
