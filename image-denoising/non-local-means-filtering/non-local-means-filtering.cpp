#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>

using namespace cv;
using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;
const string CLEAN_FILE = "flower_gray.raw";
const string NOISY_FILE = "flower_gray_noisy.raw";

Mat readRawImage(const string& filename, int width, int height) {
    Mat img(height, width, CV_8UC1);
    ifstream file(filename, ios::binary);
    file.read(reinterpret_cast<char*>(img.data), width * height);
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
    double mse = s.val[0] / (double)(original.total());

    if(mse == 0) return 100.0;

    double max_pixel = 255.0;
    return 10.0 * log10((max_pixel * max_pixel) / mse);
}

int main() {
    Mat img_original = readRawImage(CLEAN_FILE, WIDTH, HEIGHT);
    Mat img_noisy = readRawImage(NOISY_FILE, WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) return -1;

    cout << fixed << setprecision(2);
    cout << "Baseline (Noisy) PSNR: " << calculatePSNR(img_original, img_noisy) << " dB" << endl << endl;

    float default_h = 10.0;
    int default_template = 7;
    int default_search = 21;

    cout << "--- Filter Strength (h) ---" << endl;
    cout << "Fixed: Patch=7, Search=21" << endl;
    vector<float> h_values = {3, 5, 10, 15, 20, 25,30,35};
    
    double best_psnr = 0;
    float best_h = 10;

    for (float h : h_values) {
        Mat result;
        fastNlMeansDenoising(img_noisy, result, h, default_template, default_search);
        double psnr = calculatePSNR(img_original, result);
        
        cout << "h=" << setw(2) << h << " -> PSNR: " << psnr << " dB";
        if (psnr > best_psnr) {
            best_psnr = psnr;
            best_h = h;
        }
        cout << endl;
    }
    cout << endl;

    cout << "--- Patch Size N' (Template Window) ---" << endl;
    cout << "Fixed: h=" << best_h << ", Search=21" << endl;
    vector<int> template_sizes = {3, 5, 7, 9, 11}; // Must be odd

    for (int t : template_sizes) {
        Mat result;
        fastNlMeansDenoising(img_noisy, result, best_h, t, default_search);
        double psnr = calculatePSNR(img_original, result);
        cout << "Patch Size=" << setw(2) << t << " -> PSNR: " << psnr << " dB" << endl;
    }
    cout << endl;

    cout << "--- Search Window Size Aleph ---" << endl;
    cout << "Fixed: h=" << best_h << ", Patch=7" << endl;
    vector<int> search_sizes = {11, 21, 31, 41};

    for (int s : search_sizes) {
        Mat result;
        double t = (double)getTickCount();
        
        fastNlMeansDenoising(img_noisy, result, best_h, default_template, s);
        
        t = ((double)getTickCount() - t) / getTickFrequency();
        double psnr = calculatePSNR(img_original, result);
        
        cout << "Search Size=" << setw(2) << s << " -> PSNR: " << psnr << " dB" << " | Time: " << t << " sec" << endl;
    }
    cout << endl;

    return 0;
}
