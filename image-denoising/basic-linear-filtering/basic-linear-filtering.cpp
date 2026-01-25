#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream> // Required for file I/O
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;

Mat readRawImage(const string& filename, int width, int height) {
    Mat img(height, width, CV_8UC1);

    ifstream file(filename, ios::binary);
    
    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return Mat(); // Return empty Mat on error
    }

    file.read(reinterpret_cast<char*>(img.data), width * height);

    if (!file) {
        cerr << "Error: File size does not match expected dimensions for " << filename << endl;
        return Mat();
    }

    file.close();
    return img;
}

double calculatePSNR(const Mat& original, const Mat& denoised) {
    Mat o_float, d_float;
    original.convertTo(o_float, CV_32F);
    denoised.convertTo(d_float, CV_32F);

    Mat diff;
    absdiff(o_float, d_float, diff);
    diff = diff.mul(diff);

    Scalar s = sum(diff);
    double sse = s.val[0];
    double mse = sse / (double)(original.total());

    if (mse <= 1e-10) return 0;

    double max_pixel = 255.0;
    double psnr = 10.0 * log10((max_pixel * max_pixel) / mse);

    return psnr;
}

int main() {
    // 1. Load RAW Images
    string clean_path = "flower_gray.raw";
    string noisy_path = "flower_gray_noisy.raw"; // Assuming the noisy file is also raw

    Mat img_original = readRawImage(clean_path, WIDTH, HEIGHT);
    Mat img_noisy = readRawImage(noisy_path, WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) {
        return -1;
    }

    vector<int> kernel_sizes = {3, 5, 7, 9};

    cout << "--- Initial Check ---" << endl;
    cout << "Image Size: " << img_original.cols << "x" << img_original.rows << endl;
    cout << "PSNR of Noisy Input: " << calculatePSNR(img_original, img_noisy) << " dB" << endl << endl;

    cout << "--- Uniform Filter Results ---" << endl;
    for (int k : kernel_sizes) {
        Mat result;
        blur(img_noisy, result, Size(k, k)); 
        
        double score = calculatePSNR(img_original, result);
        cout << "Kernel [" << k << "x" << k << "] -> PSNR: " << score << " dB" << endl;
    }
    cout << endl;

    cout << "--- Gaussian Filter Results ---" << endl;
    for (int k : kernel_sizes) {
        Mat result;
        GaussianBlur(img_noisy, result, Size(k, k), 0); 
        
        double score = calculatePSNR(img_original, result);
        cout << "Kernel [" << k << "x" << k << "] -> PSNR: " << score << " dB" << endl;
    }

    return 0;
}
