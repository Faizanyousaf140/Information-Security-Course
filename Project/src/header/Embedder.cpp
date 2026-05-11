#include "Embedder.h"
#include <fstream>
#include <sstream>
#include <iostream>

Embedder::Embedder(const std::string& videoPath, const std::string& dataPath, const std::string& key)
	: video_path(videoPath), data_path(dataPath), key(key) {
}

unsigned int Embedder::hashKey(const std::string& key) {
	unsigned int hash = 0;
	for (char c : key) {
		hash = hash * 31 + c;
	}
	return hash;
}

std::string Embedder::readDataAsBits(const std::string& filepath) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) {
		return "";
	}

	std::stringstream bits;
	char byte;
	while (file.get(byte)) {
		for (int i = 7; i >= 0; --i) {
			bits << ((byte >> i) & 1);
		}
	}
	return bits.str();
}

void Embedder::embedBitsInFrame(cv::Mat& frame, const std::string& bits, int& bitIndex, int bitsToEmbed, unsigned int& seed) {
	int rows = frame.rows;
	int cols = frame.cols;
	int totalPixels = rows * cols;

	for (int i = 0; i < bitsToEmbed && bitIndex < bits.length(); ++i) {
		// Simple LCG random number generator
		seed = (seed * 1103515245 + 12345) & 0x7fffffff;
		int pixelIndex = seed % totalPixels;

		int row = pixelIndex / cols;
		int col = pixelIndex % cols;

		// Pick random channel (0=B, 1=G, 2=R)
		seed = (seed * 1103515245 + 12345) & 0x7fffffff;
		int channel = seed % 3;

		// Embed bit in LSB
		cv::Vec3b& pixel = frame.at<cv::Vec3b>(row, col);
		pixel[channel] = (pixel[channel] & 0xFE) | (bits[bitIndex] - '0');

		bitIndex++;
	}
}

bool Embedder::embedData(const std::string& outputPath) {
	// Read video
	cv::VideoCapture cap(video_path);
	if (!cap.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
		return false;
	}

	// Get video properties
	int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
	int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
	int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

	// Read data as bits
	std::string dataBits = readDataAsBits(data_path);
	if (dataBits.empty()) {
		std::cerr << "Error reading data file" << std::endl;
		return false;
	}

	// Calculate bits per frame (excluding first frame)
	int usableFrames = totalFrames - 1;
	int bitsPerFrame = (dataBits.length() + usableFrames - 1) / usableFrames;
	int paddingBits = (bitsPerFrame * usableFrames) - dataBits.length();

	// Pad data with zeros
	dataBits.append(paddingBits, '0');

	// Create metadata for first frame (32 bits for padding, 32 bits for bitsPerFrame)
	std::string metadata;
	for (int i = 31; i >= 0; --i) {
		metadata += ((paddingBits >> i) & 1) + '0';
	}
	for (int i = 31; i >= 0; --i) {
		metadata += ((bitsPerFrame >> i) & 1) + '0';
	}

	// Initialize video writer
	cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(width, height));
	if (!writer.isOpened()) {
		std::cerr << "Error creating output video" << std::endl;
		return false;
	}

	unsigned int seed = hashKey(key);
	int bitIndex = 0;
	int frameCount = 0;

	cv::Mat frame;
	while (cap.read(frame)) {
		if (frameCount == 0) {
			// First frame: embed metadata
			unsigned int metadataSeed = hashKey(key);
			int metadataIndex = 0;
			embedBitsInFrame(frame, metadata, metadataIndex, 64, metadataSeed);
		}
		else {
			// Other frames: embed data
			embedBitsInFrame(frame, dataBits, bitIndex, bitsPerFrame, seed);
		}

		writer.write(frame);
		frameCount++;
	}

	cap.release();
	writer.release();

	std::cout << "Embedding complete!" << std::endl;
	std::cout << "Total bits embedded: " << dataBits.length() << std::endl;
	std::cout << "Bits per frame: " << bitsPerFrame << std::endl;
	std::cout << "Padding bits: " << paddingBits << std::endl;

	return true;
}