#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

// --- Configuration ---
const int WIDTH = 768;
const int HEIGHT = 512;
const int CHANNELS = 3; // RGB Image

// --- Helper: Read Color RAW Image ---
// Reads 768x512x3 bytes. Assumes Interleaved RGB (R, G, B, R, G, B...)
Mat readRawImageRGB(const string& filename, int width, int height) {
    // CV_8UC3 means 8-bit Unsigned, 3 Channels
    Mat img(height, width, CV_8UC3);
    
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return Mat();
    }
    
    // Read strictly width * height * 3 bytes
    file.read(reinterpret_cast<char*>(img.data), width * height * 3);
    
    if (!file) {
        cerr << "Warning: File size might be smaller than expected!" << endl;
    }
    file.close();
    
    // Note: If the raw data is Planar (RRR...GGG...BBB), you would need a different read method.
    // Standard .raw for this course is usually Interleaved (RGBRGB...), but if colors look weird,
    // convert from BGR to RGB: cvtColor(img, img, COLOR_RGB2BGR);
    return img;
}

// --- Helper: Calculate PSNR for Color Images ---
// Formula: Average MSE across R, G, B channels, then calculate PSNR
double calculatePSNR_RGB(const Mat& original, const Mat& denoised) {
    Mat o_float, d_float;
    original.convertTo(o_float, CV_32F);
    denoised.convertTo(d_float, CV_32F);

    Mat diff;
    absdiff(o_float, d_float, diff); // |Y - X|
    diff = diff.mul(diff);           // (Y - X)^2

    Scalar s = sum(diff); // Sum of all channels
    
    // Total number of values = Width * Height * 3
    double sse = s.val[0] + s.val[1] + s.val[2];
    double mse = sse / (double)(original.total() * original.channels());

    if (mse <= 1e-10) return 0;

    double max_pixel = 255.0;
    double psnr = 10.0 * log10((max_pixel * max_pixel) / mse);

    return psnr;
}

int main() {
    // 1. Load Color Images
    // Assuming filenames are flower.raw and flower_noisy.raw based on the prompt
    string clean_path = "flower.raw"; 
    string noisy_path = "flower_noisy.raw"; 

    Mat img_original = readRawImageRGB(clean_path, WIDTH, HEIGHT);
    Mat img_noisy = readRawImageRGB(noisy_path, WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) return -1;
    
    // Convert RGB to BGR for proper display in OpenCV (OpenCV uses BGR by default)
    // If your RAW is RGB, this swaps it to BGR. If RAW is BGR, comment this out.
    cvtColor(img_original, img_original, COLOR_RGB2BGR);
    cvtColor(img_noisy, img_noisy, COLOR_RGB2BGR);

    cout << "--- Initial Check ---" << endl;
    cout << "Noisy Image PSNR: " << calculatePSNR_RGB(img_original, img_noisy) << " dB" << endl << endl;

    // --- STEP 1: Remove Impulse Noise (Median Filter) ---
    Mat img_step1;
    // Kernel size 3 is usually sufficient for S&P noise. 5 might be too aggressive.
    medianBlur(img_noisy, img_step1, 3);
    
    double psnr_step1 = calculatePSNR_RGB(img_original, img_step1);
    cout << "Step 1 (Median Filter 3x3) PSNR: " << psnr_step1 << " dB" << endl;

    // --- STEP 2: Remove Gaussian Noise (Bilateral Filter) ---
    Mat img_final;
    // d: Diameter of pixel neighborhood (9 is common)
    // sigmaColor: 75 (mixes colors that are somewhat different)
    // sigmaSpace: 75 (mixes pixels that are spatially close)
    bilateralFilter(img_step1, img_final, 9, 75, 75);
    
    double psnr_final = calculatePSNR_RGB(img_original, img_final);
    cout << "Step 2 (Bilateral Filter) PSNR:  " << psnr_final << " dB" << endl;

    // Save results
    imwrite("flower_step1_median.png", img_step1);
    imwrite("flower_step2_final.png", img_final);

    cout << "\nResults saved as 'flower_step1_median.png' and 'flower_step2_final.png'." << endl;

    return 0;
}
