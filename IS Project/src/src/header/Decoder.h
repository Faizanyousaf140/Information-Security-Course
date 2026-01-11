#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class Decoder {

private:
	std::string video_path;
	unsigned int seed;

	// Helper methods
	std::string extractBitsFromFrame(cv::Mat& frame, int pixelsToExtract, unsigned int& rngSeed);

public:
	Decoder(const std::string& videoPath, unsigned int seed);

	std::string decodeData();
};