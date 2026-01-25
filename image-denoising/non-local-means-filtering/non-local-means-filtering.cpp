#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>

using namespace cv;
using namespace std;

// --- Configuration ---
const int WIDTH = 768;
const int HEIGHT = 512;
const string CLEAN_FILE = "flower_gray.raw";
const string NOISY_FILE = "flower_gray_noisy.raw";

// --- Helper: Read RAW Image ---
Mat readRawImage(const string& filename, int width, int height) {
    Mat img(height, width, CV_8UC1);
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return Mat();
    }
    file.read(reinterpret_cast<char*>(img.data), width * height);
    file.close();
    return img;
}

// --- Helper: Calculate PSNR ---
double calculatePSNR(const Mat& original, const Mat& denoised) {
    Mat o_float, d_float;
    original.convertTo(o_float, CV_32F);
    denoised.convertTo(d_float, CV_32F);

    Mat diff;
    absdiff(o_float, d_float, diff);
    diff = diff.mul(diff);

    Scalar s = sum(diff);
    double mse = s.val[0] / (double)(original.total());
    if (mse <= 1e-10) return 0;

    double max_pixel = 255.0;
    return 10.0 * log10((max_pixel * max_pixel) / mse);
}

int main() {
    // 1. Load Data
    Mat img_original = readRawImage(CLEAN_FILE, WIDTH, HEIGHT);
    Mat img_noisy = readRawImage(NOISY_FILE, WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) return -1;

    cout << fixed << setprecision(2);
    cout << "--- Baseline ---" << endl;
    cout << "Noisy Image PSNR: " << calculatePSNR(img_original, img_noisy) << " dB" << endl << endl;

    // 2. Experiment with NLM Parameters
    // OpenCV fastNlMeansDenoising parameters mapping:
    // h: Filtering strength (the 'h' in the formula)
    // templateWindowSize: Size of the patch (small neighbor window, 'N_prime' or patch size)
    // searchWindowSize: Size of the search area (big search window, 'Aleph')
    
    // We will vary 'h' to find the sweet spot, keeping window sizes constant initially
    vector<float> h_values = {5, 10, 15, 20, 25};
    int templateWindowSize = 7; // Common default (7x7 patch)
    int searchWindowSize = 21;  // Common default (21x21 search area)

    cout << "--- NLM Parameter Experiment (Varying h) ---" << endl;
    double best_psnr = 0;
    float best_h = 0;

    for (float h : h_values) {
        Mat result;
        // fastNlMeansDenoising(src, dst, h, templateWindowSize, searchWindowSize)
        fastNlMeansDenoising(img_noisy, result, h, templateWindowSize, searchWindowSize);
        
        double score = calculatePSNR(img_original, result);
        cout << "h=" << h << ", PatchSize=" << templateWindowSize 
             << ", SearchSize=" << searchWindowSize 
             << " -> PSNR: " << score << " dB" << endl;

        if (score > best_psnr) {
            best_psnr = score;
            best_h = h;
        }
    }
    cout << endl;

    // 3. Analyze effect of Patch Size (Template Window) using the best h
    cout << "--- NLM Parameter Experiment (Varying Patch Size) ---" << endl;
    vector<int> patch_sizes = {3, 5, 7, 9};
    for (int p : patch_sizes) {
        Mat result;
        fastNlMeansDenoising(img_noisy, result, best_h, p, searchWindowSize);
        double score = calculatePSNR(img_original, result);
        cout << "h=" << best_h << ", PatchSize=" << p 
             << ", SearchSize=" << searchWindowSize 
             << " -> PSNR: " << score << " dB" << endl;
    }

    // 4. Final Best Result
    cout << "\n--- Final Recommendation ---" << endl;
    cout << "Based on experiments, a good choice is likely h=" << best_h 
         << " with PatchSize=7." << endl;

    // Optional: Save output
    // Mat final_result;
    // fastNlMeansDenoising(img_noisy, final_result, best_h, 7, 21);
    // imwrite("nlm_result.png", final_result);

    return 0;
}