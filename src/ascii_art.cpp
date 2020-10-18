#include "ascii_art.h"
#include "ansi.h"

#include <iostream>

ASCIIArt::ASCIIArt(std::vector<std::string> art_lines) {
	this->art_lines = art_lines;				 
}

std::string ASCIIArt::pad_to_length(std::string data, int length) {
	int padding_length = length - data.length();
	return data + std::string(padding_length, ' ');
}

void ASCIIArt::print(std::vector<std::string> data) {
	int line_count = this->art_lines.size();
	int data_count = data.size();
	for (int i = 0; i < data_count; i++) {
		std::cout << BRIGHT_CYAN_COLOR;
		if (i < line_count) {
				std::cout << this->pad_to_length(this->art_lines[i], 41) + data[i] << std::endl;
		} else {
				std::cout << this->pad_to_length("", 41) + data[i] << std::endl;
		}
	}
}
