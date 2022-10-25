#include "common/Common.h"

std::string trim(const std::string &s, const std::string& delims)
{
	static const std::string default_delims = "\t\r\n ";
	std::string append_delims = default_delims + delims;
	std::string::size_type last = s.find_last_not_of(append_delims);
	if(last != std::string::npos)
	{
		return s.substr(s.find_first_not_of(append_delims), last+1);
	}
	return s;
}

void strSplit(const std::string &s, const std::string &delimiters, std::vector<std::string> &tokens)
{
	std::string str = trim(s);
	std::size_t pos = str.find(delimiters);
	while(pos != std::string::npos)
	{
		tokens.push_back(trim(str.substr(0, pos)));
		str = trim(str.substr(pos+delimiters.length()));
		pos = str.find(delimiters);
	}

	if(str.length() <= 0)
		return;
	
	tokens.push_back(str);
}