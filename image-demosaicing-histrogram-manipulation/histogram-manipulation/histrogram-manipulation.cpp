#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <cmath>

const int WIDTH = 1024;
const int HEIGHT = 1024;
const int NUM_PIXELS = WIDTH * HEIGHT;
const int GRAY_LEVELS = 256;

struct PixelInfo {
    unsigned char value;
    int originalIndex;
};

std::vector<unsigned char> readRaw(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Could not open " << filename << std::endl;
        exit(1);
    }
    std::vector<unsigned char> img(std::istreambuf_iterator<char>(inFile), {});
    inFile.close();
    
    if (img.size() != NUM_PIXELS) {
        std::cerr << "Warning: File size (" << img.size() << ") != " 
                  << WIDTH << "x" << HEIGHT << ". Check dimensions!" << std::endl;
    }
    return img;
}

void writeRaw(const std::string& filename, const std::vector<unsigned char>& img) {
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(img.data()), img.size());
    outFile.close();
    std::cout << "Saved: " << filename << std::endl;
}

void saveCSV(const std::string& filename, const std::vector<int>& data, const std::string& header) {
    std::ofstream file(filename);
    file << header << "\n";
    for (int i = 0; i < data.size(); ++i) {
        file << i << "," << data[i] << "\n";
    }
    file.close();
    std::cout << "Data saved for plotting: " << filename << std::endl;
}

std::vector<int> computeHistogram(const std::vector<unsigned char>& img) {
    std::vector<int> hist(GRAY_LEVELS, 0);
    for (unsigned char val : img) {
        hist[val]++;
    }
    return hist;
}

std::vector<int> computeCDF(const std::vector<int>& hist) {
    std::vector<int> cdf(GRAY_LEVELS, 0);
    cdf[0] = hist[0];
    for (int i = 1; i < GRAY_LEVELS; ++i) {
        cdf[i] = cdf[i - 1] + hist[i];
    }
    return cdf;
}

std::vector<unsigned char> methodA(const std::vector<unsigned char>& inputImg) {
    std::vector<int> hist = computeHistogram(inputImg);
    std::vector<int> cdf = computeCDF(hist);
    
    std::vector<int> transferFunc(GRAY_LEVELS);
    for (int i = 0; i < GRAY_LEVELS; ++i) {
        transferFunc[i] = std::round((float)(GRAY_LEVELS - 1) * cdf[i] / NUM_PIXELS);
    }

    saveCSV("methodA_transfer_function.csv", transferFunc, "Input_Intensity,Output_Intensity");

    std::vector<unsigned char> outputImg(NUM_PIXELS);
    for (int i = 0; i < NUM_PIXELS; ++i) {
        outputImg[i] = static_cast<unsigned char>(transferFunc[inputImg[i]]);
    }
    
    return outputImg;
}

std::vector<unsigned char> methodB(const std::vector<unsigned char>& inputImg) {
    std::vector<PixelInfo> pixels(NUM_PIXELS);
    for (int i = 0; i < NUM_PIXELS; ++i) {
        pixels[i] = { inputImg[i], i };
    }

    std::sort(pixels.begin(), pixels.end(), [](const PixelInfo& a, const PixelInfo& b) {
        return a.value < b.value;
    });

    std::vector<unsigned char> outputImg(NUM_PIXELS);
    
    for (int i = 0; i < NUM_PIXELS; ++i) {
        int newVal = (i * GRAY_LEVELS) / NUM_PIXELS;
        
        if (newVal > 255) newVal = 255;

        outputImg[pixels[i].originalIndex] = (unsigned char)(newVal);
    }

    return outputImg;
}

int main() {
    std::string filename = "airplane.raw";
    
    std::vector<unsigned char> img = readRaw(filename);

    std::vector<int> originalHist = computeHistogram(img);
    saveCSV("original_histogram.csv", originalHist, "Intensity,Pixel_Count");

    std::vector<unsigned char> imgA = methodA(img);
    writeRaw("airplane_methodA.raw", imgA);

    std::vector<unsigned char> imgB = methodB(img);
    writeRaw("airplane_methodB.raw", imgB);

    std::vector<int> histB = computeHistogram(imgB);
    std::vector<int> cdfB = computeCDF(histB);
    saveCSV("methodB_cdf.csv", cdfB, "Intensity,Cumulative_Count");

    std::cout << "Done! All images and CSV data files generated." << std::endl;
    return 0;
}
