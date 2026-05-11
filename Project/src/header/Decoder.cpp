#include "Decoder.h"
#include <iostream>
#include <set>

Decoder::Decoder(const std::string& videoPath, unsigned int seed)
	: video_path(videoPath), seed(seed) {
}

std::string Decoder::extractBitsFromFrame(cv::Mat& frame, int pixelsToExtract, unsigned int& rngSeed) {
	std::string bits;
	int rows = frame.rows;
	int cols = frame.cols;
	int totalPixels = rows * cols;

	std::cout << "Extracting " << pixelsToExtract << " pixels from frame of size " << rows << "x" << cols << std::endl;

	std::set<int> usedPixels;

	for (int i = 0; i < pixelsToExtract; ++i) {
		// Generate unique random pixel index (same logic as embedder)
		int pixelIndex;
		do {
			rngSeed = 1 + i;//(rngSeed * 1103515245 + 12345) & 0x7fffffff;
			pixelIndex = rngSeed;//% totalPixels;
		} while (usedPixels.count(pixelIndex));

		std::cout << "Pixel Index: " << pixelIndex << std::endl;
		usedPixels.insert(pixelIndex);

		int row = pixelIndex / cols;
		int col = pixelIndex % cols;
		std::cout << "Extracting from pixel (" << row << ", " << col << ")" << std::endl;

		// Extract all 3 RGB channels
		cv::Vec3b pixel = frame.at<cv::Vec3b>(row, col);
		for (int channel = 0; channel < 3; ++channel) {
			bits += ((pixel[channel] & 1) + '0');
		}
	}

	std::cout << "Extracted bits: " << bits << std::endl;
	return bits;
}

std::string Decoder::decodeData() {
	// Open video
	cv::VideoCapture cap(video_path);
	if (!cap.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
		return "";
	}

	cv::Mat frame;
	int frameCount = 0;

	int paddingBits = 0;
	int pixelsPerFrame = 0;
	std::string allDataBits;

	unsigned int rngSeed = seed;

	while (cap.read(frame)) {
		if (frameCount == 0) {
			std::cout << "Extracting metadata from first frame..." << std::endl;
			// First frame: extract metadata (64 bits = ~22 pixels)
			unsigned int metadataSeed = seed;
			std::string metadata = extractBitsFromFrame(frame, 22, metadataSeed);

			// Extract padding bits (first 32 bits)
			paddingBits = 0;
			for (int i = 0; i < 32; ++i) {
				paddingBits = (paddingBits << 1) | (metadata[i] - '0');
			}

			// Extract pixels per frame (next 32 bits)
			pixelsPerFrame = 0;
			for (int i = 32; i < 64; ++i) {
				pixelsPerFrame = (pixelsPerFrame << 1) | (metadata[i] - '0');
			}

			std::cout << "Metadata extracted:" << std::endl;
			std::cout << "Pixels per frame: " << pixelsPerFrame << std::endl;
			std::cout << "Padding bits: " << paddingBits << std::endl;
			pixelsPerFrame = 1;
			paddingBits = 594;
		}
		else {
			// Other frames: extract data
			std::string frameBits = extractBitsFromFrame(frame, pixelsPerFrame, rngSeed);
			allDataBits += frameBits;
		}

		frameCount++;
	}

	cap.release();

	// Remove padding bits
	if (paddingBits > 0 && paddingBits < allDataBits.length()) {
		allDataBits = allDataBits.substr(0, allDataBits.length() - paddingBits);
	}

	std::cout << "Decoding complete!" << std::endl;
	std::cout << "Total bits extracted: " << allDataBits.length() << std::endl;

	return allDataBits;
}