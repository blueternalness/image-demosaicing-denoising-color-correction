#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iomanip>

using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;
const int MAX_VAL = 255;


// Mirroring for boundary condition
/*
unsigned char getPixel(const vector<unsigned char>& data, int x, int y) {
    // If x is -1, it becomes 1. If x is WIDTH, it becomes WIDTH - 2.
    if (x < 0) x = -x;
    if (x >= WIDTH) x = 2 * WIDTH - x - 2;
    
    if (y < 0) y = -y;
    if (y >= HEIGHT) y = 2 * HEIGHT - y - 2;

    return data[y * WIDTH + x];
}
*/


// Zero Padding for boundary condition
/*
unsigned char getPixel(const vector<unsigned char>& data, int x, int y) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return 0;
    return data[y * WIDTH + x];
}
*/

double calculatePSNR(const vector<unsigned char>& original, const vector<unsigned char>& filtered) {
    double mse = 0;
    for (int i = 0; i < WIDTH * HEIGHT; ++i) {
        double diff = (double)original[i] - (double)filtered[i];
        mse += diff * diff;
    }
    mse /= (WIDTH * HEIGHT);

    return 10.0 * log10((MAX_VAL * MAX_VAL) / mse);
}

// Clamping
unsigned char getPixel(const vector<unsigned char>& data, int x, int y) {
    int nx = max(0, min(x, WIDTH - 1));
    int ny = max(0, min(y, HEIGHT - 1));
    return data[ny * WIDTH + nx];
}

double getTheoreticalSigma(int size) {
    return 0.3 * ((size - 1) * 0.5 - 1) + 0.8;
}

vector<unsigned char> applyUniformFilter(const vector<unsigned char>& input, int size) {
    vector<unsigned char> output(WIDTH * HEIGHT);
    int offset = size / 2;
    double area = (double)(size * size);

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double sum = 0;
            for (int ky = -offset; ky <= offset; ++ky) {
                for (int kx = -offset; kx <= offset; ++kx) {
                    sum += getPixel(input, x + kx, y + ky);
                }
            }
            // add 0.5 for rounding
            output[y * WIDTH + x] = (unsigned char)(sum / area + 0.5);
        }
    }
    return output;
}

vector<unsigned char> applyGaussianFilter(const vector<unsigned char>& input, int size, double sigma) {
    vector<unsigned char> output(WIDTH * HEIGHT);
    int offset = size / 2;
    vector<vector<double>> kernel(size, vector<double>(size));
    double sum_kernel = 0;

    for (int i = -offset; i <= offset; ++i) {
        for (int j = -offset; j <= offset; ++j) {
            double val = exp(-(i * i + j * j) / (2 * sigma * sigma));
            kernel[i + offset][j + offset] = val;
            sum_kernel += val;
        }
    }

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double res = 0;
            for (int ky = -offset; ky <= offset; ++ky) {
                for (int kx = -offset; kx <= offset; ++kx) {
                    res += getPixel(input, x + kx, y + ky) * kernel[ky + offset][kx + offset];
                }
            }
            // add 0.5 for rounding
            output[y * WIDTH + x] = (unsigned char)((res / sum_kernel) + 0.5);
        }
    }
    return output;
}

int main() {
    vector<unsigned char> original(WIDTH * HEIGHT);
    vector<unsigned char> noisy(WIDTH * HEIGHT);

    ifstream raw_orig("flower_gray.raw", ios::binary);
    ifstream raw_noisy("flower_gray_noisy.raw", ios::binary);

    raw_orig.read((char*)original.data(), WIDTH * HEIGHT);
    raw_noisy.read((char*)noisy.data(), WIDTH * HEIGHT);

    cout << fixed << setprecision(5);
    cout << "Initial Noisy PSNR: " << calculatePSNR(original, noisy) << " dB" << endl;
    for (int kernel_size : {3, 5, 7, 9, 15}) {
        
        auto uniform = applyUniformFilter(noisy, kernel_size);
        double psnr_uniform = calculatePSNR(original, uniform);

        double sigma = getTheoreticalSigma(kernel_size);
        auto gaussianTheoreticalSigma = applyGaussianFilter(noisy, kernel_size, sigma);
        double psnr_gaussian = calculatePSNR(original, gaussianTheoreticalSigma);


        auto gaussianLargeSigma = applyGaussianFilter(noisy, kernel_size, 100.0); 
        double psnr_gaussian_large = calculatePSNR(original, gaussianLargeSigma);

        cout << "Kernel " << kernel_size << "x" << kernel_size << " | Sigma: " << sigma << endl;
        cout << "  Uniform PSNR:   " << psnr_uniform << " dB" << endl;
        cout << "  Gaussian PSNR with Theoretical Sigma:  " << psnr_gaussian << " dB" << endl;
        cout << "  Gaussian PSNR with Large Sigma:  " << psnr_gaussian_large << " dB" << endl;
        
    }

    return 0;
}