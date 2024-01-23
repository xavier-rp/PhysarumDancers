#pragma once
#include <vector>

#include "Grid.hpp"

class TrailMap {
public:
	int width;
	int height;
	std::vector<std::vector<float>> matrix;

	TrailMap(int width, int height) :
		width{ width },
		height{ height } {
		matrix = std::vector<std::vector<float>>( height, std::vector<float>(width, 0.0) );
	}



};