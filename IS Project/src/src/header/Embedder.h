#pragma once
#include <string>
#include <opencv2/opencv.hpp>

class Embedder {

private:
	std::string video_path;
	std::string data_path;
	std::string key;

	// Helper methods
	unsigned int hashKey(const std::string& key);
	void embedBitsInFrame(cv::Mat& frame, const std::string& bits, int& bitIndex, int bitsToEmbed, unsigned int& seed);
	std::string readDataAsBits(const std::string& filepath);

public:
	Embedder(const std::string& videoPath, const std::string& dataPath, const std::string& key);

	bool embedData(const std::string& outputPath);
};