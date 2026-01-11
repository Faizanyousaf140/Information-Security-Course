#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class Encoder {

private:
	std::string video_path;
	std::string data_bits;
	unsigned int seed;

	// Helper methods
	void embedBitsInFrame(cv::Mat& frame, const std::string& bits, int& bitIndex, int pixelsToModify, unsigned int& rngSeed);

public:
	Encoder(const std::string& videoPath, const std::string& dataBits, unsigned int seed);

	bool embedData(const std::string& outputPath);
};