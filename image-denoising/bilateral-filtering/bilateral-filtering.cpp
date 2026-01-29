#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;

inline unsigned char getPixel(const vector<unsigned char>& data, int x, int y, int width, int height) {
    int c = max(0, min(x, width - 1));
    int r = max(0, min(y, height - 1));
    return data[r * width + c];
}

vector<unsigned char> readRawImage(const string& filename, int width, int height) {
    vector<unsigned char> img(width * height);
    ifstream file(filename, ios::binary);
    file.read(reinterpret_cast<char*>(img.data()), width * height);
    file.close();

    return img;
}

double calculatePSNR(const vector<unsigned char>& original, const vector<unsigned char>& denoised) {
    double mse = 0.0;
    long total_pixels = original.size();
    for (size_t i = 0; i < total_pixels; ++i) {
        double diff = (double)original[i] - (double)denoised[i];
        mse += diff * diff;
    }
    mse /= (double)total_pixels;
    if (mse == 0) return 100.0;
    return 10.0 * log10((255.0 * 255.0) / mse);
}

void applyBilateralFilter(const vector<unsigned char>& src, vector<unsigned char>& dst, 
                          int width, int height, 
                          int kernel_radius, double sigma_c, double sigma_s) {
    dst.resize(src.size());
    double two_sigma_c_sq = 2 * sigma_c * sigma_c;
    double two_sigma_s_sq = 2 * sigma_s * sigma_s;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            double sum_weights = 0.0;
            double sum_pixel_values = 0.0;
            double center_intensity = (double)getPixel(src, j, i, width, height);

            for (int m = -kernel_radius; m <= kernel_radius; m++) {
                for (int n = -kernel_radius; n <= kernel_radius; n++) {
                    int neighbor_x = j + n;
                    int neighbor_y = i + m;
                    double neighbor_intensity = (double)getPixel(src, neighbor_x, neighbor_y, width, height);

                    double spatial_dist_sq = m*m + n*n;
                    double intensity_diff = center_intensity - neighbor_intensity;
                    double intensity_diff_sq = intensity_diff * intensity_diff;

                    double term_spatial = spatial_dist_sq / two_sigma_c_sq;
                    double term_intensity = intensity_diff_sq / two_sigma_s_sq;
                    double weight = exp(-(term_spatial + term_intensity));

                    sum_pixel_values += neighbor_intensity * weight;
                    sum_weights += weight;
                }
            }
            double result_val = sum_pixel_values / sum_weights;
            if (result_val < 0.0) result_val = 0.0;
            if (result_val > 255.0) result_val = 255.0;
            dst[i * width + j] = (unsigned char)(result_val + 0.5);
        }
    }
}

struct TestResult {
    double sigma_c;
    double sigma_s;
    double psnr;
};

int main() {
    vector<unsigned char> img_original = readRawImage("flower_gray.raw", WIDTH, HEIGHT);
    vector<unsigned char> img_noisy = readRawImage("flower_gray_noisy.raw", WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) return -1;

    vector<unsigned char> result_img;
    
    int kernel_radius = 2; 
    
    vector<double> sigma_c_values = { 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 8.0, 10.0, 20.0, 40.0, 80.0, 150.0 }; 
    vector<double> sigma_s_values = { 10.0, 30.0, 40.0, 50.0, 60.0, 70.0, 80.0, 90.0, 100.0, 150.0, 300.0 };

    vector<TestResult> results;
    TestResult best_result = {0, 0, -1.0};

    cout << "--- 5x5 ---" << endl;
    cout << "Sigma C|Sigma S|PSNR (dB)|" << endl;

    for (double sc : sigma_c_values) {
        for (double ss : sigma_s_values) {
            
            applyBilateralFilter(img_noisy, result_img, WIDTH, HEIGHT, kernel_radius, sc, ss);
            double psnr = calculatePSNR(img_original, result_img);

            TestResult current = {sc, ss, psnr};
            results.push_back(current);

            if (psnr > best_result.psnr) best_result = current;

            cout << "| " << sc << " | " << ss << " | " << psnr << " |" << endl;
        }
    }

    cout << "Best 5x5 Config: Sigma C=" << best_result.sigma_c << ", Sigma S=" << best_result.sigma_s << "PSNR: " << best_result.psnr << " dB" << endl;

    return 0;
}