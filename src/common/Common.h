#ifndef __H__COMMON__H__
#define __H__COMMON__H__

#include <string>
#include <vector>

std::string trim(const std::string &s, const std::string& delims = "");
void strSplit(const std::string &s, const std::string &delimiters, std::vector<std::string> &tokens);

#endif