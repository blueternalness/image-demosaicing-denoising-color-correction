#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>

using namespace cv;
using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;

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
    if (mse <= 1e-10) return 0;

    double max_pixel = 255.0;
    return 10.0 * log10((max_pixel * max_pixel) / mse);
}

void applyBilateralFilter(const Mat& src, Mat& dst, int kernel_radius, double sigma_c, double sigma_s) {
    dst = src.clone();
    
    int width = src.cols;
    int height = src.rows;
    
    for (int i = kernel_radius; i < height - kernel_radius; i++) {
        for (int j = kernel_radius; j < width - kernel_radius; j++) {
            
            double sum_weights = 0.0;
            double sum_pixel_values = 0.0;
            
            double center_intensity = (double)src.at<uchar>(i, j);

            for (int m = -kernel_radius; m <= kernel_radius; m++) {
                for (int n = -kernel_radius; n <= kernel_radius; n++) {
                    
                    int k = i + m;
                    int l = j + n;
                    
                    double neighbor_intensity = (double)src.at<uchar>(k, l);

                    double spatial_dist_sq = m*m + n*n;
                    
                    double intensity_diff = center_intensity - neighbor_intensity;
                    double intensity_diff_sq = intensity_diff * intensity_diff;

                    double term_spatial = spatial_dist_sq / (2 * sigma_c * sigma_c);
                    double term_intensity = intensity_diff_sq / (2 * sigma_s * sigma_s);
                    
                    double weight = exp(-(term_spatial + term_intensity));

                    sum_pixel_values += neighbor_intensity * weight;
                    sum_weights += weight;
                }
            }
            
            dst.at<uchar>(i, j) = saturate_cast<uchar>(sum_pixel_values / sum_weights);
        }
    }
}

int main() {
    Mat img_original = readRawImage("flower_gray.raw", WIDTH, HEIGHT);
    Mat img_noisy = readRawImage("flower_gray_noisy.raw", WIDTH, HEIGHT);

    if (img_original.empty() || img_noisy.empty()) return -1;

    Mat result;
    
    //int kernel_radius = 2;
    int kernel_radius = 15;
    //double sigma_c = 1.0;
    //double sigma_s = 30.0;
    double sigma_c = 150.0;
    double sigma_s = 150.0;

    cout << "Processing Bilateral Filter (this may take a moment)..." << endl;
    applyBilateralFilter(img_noisy, result, kernel_radius, sigma_c, sigma_s);

    double psnr = calculatePSNR(img_original, result);

    cout << "--- Results ---" << endl;
    cout << "Parameters: Window=" << (2*kernel_radius+1) << "x" << (2*kernel_radius+1);
    cout << ", Sigma_C=" << sigma_c << ", Sigma_S=" << sigma_s << endl;
    cout << "PSNR: " << psnr << " dB" << endl;

    return 0;
}
