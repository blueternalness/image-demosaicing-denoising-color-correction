#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cmath>

using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;
const int CHANNELS = 3;
const int IMG_SIZE = WIDTH * HEIGHT * CHANNELS;

unsigned char* readRawImage(const char* filename) {
    FILE* file = fopen(filename, "rb");
    unsigned char* imageData = new unsigned char[IMG_SIZE];
    fread(imageData, sizeof(unsigned char), IMG_SIZE, file);
    fclose(file);
    return imageData;
}

void writeRawImage(const char* filename, unsigned char* imageData) {
    FILE* file = fopen(filename, "wb");
    fwrite(imageData, sizeof(unsigned char), IMG_SIZE, file);
    fclose(file);
}

double calculatePSNR(unsigned char* original, unsigned char* denoised) {
    double mse = 0.0;
    for (int i = 0; i < IMG_SIZE; ++i) {
        double diff = static_cast<double>(original[i]) - static_cast<double>(denoised[i]);
        mse += diff * diff;
    }
    mse /= IMG_SIZE;
    if (mse == 0) return 100.0;
    return 10.0 * log10((255.0 * 255.0) / mse);
}

void applyMedianFilter(unsigned char* input, unsigned char* output, int width, int height) {
    int windowSize = 3;
    int offset = windowSize / 2;

    for (int c = 0; c < CHANNELS; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                vector<unsigned char> window;
                for (int ky = -offset; ky <= offset; ++ky) {
                    for (int kx = -offset; kx <= offset; ++kx) {
                        int ny = min(max(y + ky, 0), height - 1);
                        int nx = min(max(x + kx, 0), width - 1);
                        window.push_back(input[(ny * width + nx) * CHANNELS + c]);
                    }
                }
                sort(window.begin(), window.end());
                output[(y * width + x) * CHANNELS + c] = window[window.size() / 2];
            }
        }
    }
}

void applyBilateralFilter(unsigned char* input, unsigned char* output, int width, int height, double sigma_d, double sigma_r) {
    int kernelRadius = 2; // 5x5
    
    int kernelSize = 2 * kernelRadius + 1;
    vector<double> spatialWeights(kernelSize * kernelSize);
    for (int ky = -kernelRadius; ky <= kernelRadius; ++ky) {
        for (int kx = -kernelRadius; kx <= kernelRadius; ++kx) {
            double distSq = ky * ky + kx * kx;
            spatialWeights[(ky + kernelRadius) * kernelSize + (kx + kernelRadius)] = exp(-distSq / (2 * sigma_d * sigma_d));
        }
    }

    for (int c = 0; c < CHANNELS; ++c) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                
                double sumWeights = 0.0;
                double sumValues = 0.0;
                double centerPixelVal = static_cast<double>(input[(y * width + x) * CHANNELS + c]);

                for (int ky = -kernelRadius; ky <= kernelRadius; ++ky) {
                    for (int kx = -kernelRadius; kx <= kernelRadius; ++kx) {
                        
                        int ny = min(max(y + ky, 0), height - 1);
                        int nx = min(max(x + kx, 0), width - 1);

                        double neighborPixelVal = static_cast<double>(input[(ny * width + nx) * CHANNELS + c]);
                        
                        double diff = centerPixelVal - neighborPixelVal;
                        double rangeWeight = exp(-(diff * diff) / (2 * sigma_r * sigma_r));

                        double spatialWeight = spatialWeights[(ky + kernelRadius) * kernelSize + (kx + kernelRadius)];

                        double weight = spatialWeight * rangeWeight;

                        sumValues += neighborPixelVal * weight;
                        sumWeights += weight;
                    }
                }
                
                output[(y * width + x) * CHANNELS + c] = static_cast<unsigned char>(min(max(sumValues / sumWeights, 0.0), 255.0));
            }
        }
    }
}

int main() {
    const char* originalFileName = "flower.raw";       
    const char* noisyFileName = "flower_noisy.raw";    
    const char* outputFileName = "flower_denoised_bilateral.raw"; 

    unsigned char* originalImage = readRawImage(originalFileName); 
    unsigned char* noisyImage = readRawImage(noisyFileName);
    
    unsigned char* medianFilteredImage = new unsigned char[IMG_SIZE];
    unsigned char* finalDenoisedImage = new unsigned char[IMG_SIZE];

    applyMedianFilter(noisyImage, medianFilteredImage, WIDTH, HEIGHT);

    applyBilateralFilter(medianFilteredImage, finalDenoisedImage, WIDTH, HEIGHT, 2.0, 30.0);

    writeRawImage(outputFileName, finalDenoisedImage);

    double psnrNoisy = calculatePSNR(originalImage, noisyImage);
    cout << "PSNR (Noisy Image): " << psnrNoisy << " dB" << endl;

    double psnrDenoised = calculatePSNR(originalImage, finalDenoisedImage);
    cout << "PSNR (Denoised Image): " << psnrDenoised << " dB" << endl;

    return 0;
}
