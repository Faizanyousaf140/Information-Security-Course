#include "Encoder.h"
#include <iostream>
#include <set>

Encoder::Encoder(const std::string& videoPath, const std::string& dataBits, unsigned int seed)
	: video_path(videoPath), data_bits(dataBits), seed(seed) {
}

void Encoder::embedBitsInFrame(cv::Mat& frame, const std::string& bits, int& bitIndex, int pixelsToModify, unsigned int& rngSeed) {
	int rows = frame.rows;
	int cols = frame.cols;
	int totalPixels = rows * cols;

	std::set<int> usedPixels;

	for (int i = 0; i < pixelsToModify && bitIndex < bits.length(); ++i) {
		// Generate unique random pixel index
		int pixelIndex;
		do {
			rngSeed = 1 + i;//(rngSeed * 1103515245 + 12345) & 0x7fffffff;
			pixelIndex = rngSeed;//% totalPixels;
		} while (usedPixels.count(pixelIndex));

		usedPixels.insert(pixelIndex);

		int row = pixelIndex / cols;
		int col = pixelIndex % cols;

		std::cout << "Embedding at pixel (" << row << ", " << col << ")" << std::endl;

		std::cout << "Bits: " << bits << std::endl;
		std::cout << "Bits length: " << bits.length() << std::endl;

		// Modify all 3 RGB channels
		cv::Vec3b& pixel = frame.at<cv::Vec3b>(row, col);
		for (int channel = 0; channel < 3 && bitIndex < bits.length(); ++channel) {
			pixel[channel] = (pixel[channel] & 0xFE) | (bits[bitIndex] - '0');
			//std::cout << "Embedding bit " << bits[bitIndex] << " at pixel (" << row << ", " << col << "), channel " << channel << std::endl;
			std::cout << "Bit index: " << bitIndex << " Bit: " << bits[bitIndex] << std::endl;
			bitIndex++;
		}
	}
}

bool Encoder::embedData(const std::string& outputPath) {
	// Open video
	std::cout << "Opening video file: " << video_path << std::endl;
	//return false;
	std::string pathtemp = "input.mp4";
	cv::VideoCapture cap(pathtemp);
	if (!cap.isOpened()) {
		std::cerr << "Error opening video file" << std::endl;
		return false;
	}

	std::cout << "Data length (bits): " << data_bits.length() << std::endl;
	std::cout << "Data: " << data_bits << std::endl;
	std::cout << "Starting embedding process..." << std::endl;

	// Get video properties
	int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
	int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
	int width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
	int height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

	// Calculate bits per frame (excluding first frame for data)
	int usableFrames = totalFrames - 1;
	int totalDataBits = data_bits.length();
	int bitsPerFrame = (totalDataBits + usableFrames - 1) / usableFrames;
	int paddingBits = (bitsPerFrame * usableFrames) - totalDataBits;

	std::cout << "Video properties:" << std::endl;
	std::cout << " Total Frames: " << totalFrames << std::endl;
	std::cout << " FPS: " << fps << std::endl;
	std::cout << " Width: " << width << std::endl;
	std::cout << " Height: " << height << std::endl;
	std::cout << "Usable Frames for Data: " << usableFrames << std::endl
		   << " Bits per Frame: " << bitsPerFrame << std::endl
		<< " Padding Bits: " << paddingBits << std::endl;

	// Pad data with zeros
	std::string paddedData = data_bits + std::string(paddingBits*3, '0');
	std::cout << "Padded Data Length (bits): " << paddedData.length() << std::endl;

	// Calculate pixels to modify per frame (3 bits per pixel for RGB)
	int pixelsPerFrame = (bitsPerFrame + 2) / 3;

	// Create metadata for first frame (32 bits for padding, 32 bits for pixelsPerFrame)
	std::string metadata;
	for (int i = 31; i >= 0; --i) {
		metadata += ((paddingBits >> i) & 1) + '0';
	}
	for (int i = 31; i >= 0; --i) {
		metadata += ((pixelsPerFrame >> i) & 1) + '0';
	}

	std::cout << "Metadata to embed in first frame: " << metadata << std::endl;

	// Initialize video writer
	cv::VideoWriter writer(outputPath, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(width, height));
	if (!writer.isOpened()) {
		std::cerr << "Error creating output video" << std::endl;
		return false;
	}

	unsigned int rngSeed = seed;
	int bitIndex = 0;
	int frameCount = 0;

	cv::Mat frame;
	while (cap.read(frame)) {
		if (frameCount == 0) {
			// First frame: embed metadata (64 bits = ~22 pixels)
			unsigned int metadataSeed = seed;
			int metadataIndex = 0;
			std::cout << "Embedding metadata in first frame..." << std::endl;
			std::cout << "Metadata: " << metadata << std::endl;
			std::cout << "Metadata length: " << metadata.length() << std::endl;
			embedBitsInFrame(frame, metadata, metadataIndex, 22, metadataSeed);
		}
		else if (bitIndex < paddedData.length()) {
			// Other frames: embed data
			embedBitsInFrame(frame, paddedData, bitIndex, pixelsPerFrame, rngSeed);
		}

		writer.write(frame);
		frameCount++;
	}

	cap.release();
	writer.release();

	std::cout << "Embedding complete!" << std::endl;
	std::cout << "Total bits embedded: " << totalDataBits << std::endl;
	std::cout << "Pixels modified per frame: " << pixelsPerFrame << std::endl;
	std::cout << "Padding bits: " << paddingBits << std::endl;

	return true;
}