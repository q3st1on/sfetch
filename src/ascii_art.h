#pragma once
#include <string>
#include <vector>

class ASCIIArt {
	
	std::vector<std::string> art_lines;

	std::string pad_to_length(std::string, int);	
public:
	ASCIIArt(std::vector<std::string>);
	void print(std::vector<std::string>);

};
