#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <opencv2/opencv.hpp>

const int WIDTH = 1620;
const int HEIGHT = 1080;
const int NUM_PIXELS = WIDTH * HEIGHT;

std::vector<unsigned char> readRaw(const std::string& filename) {
    std::ifstream inFile(filename, std::ios::binary);
    std::vector<unsigned char> img(std::istreambuf_iterator<char>(inFile), {});
    inFile.close();
    return img;
}

void writeRaw(const std::string& filename, const std::vector<unsigned char>& img) {
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(reinterpret_cast<const char*>(img.data()), img.size());
    outFile.close();
}

struct YUV {
    std::vector<unsigned char> Y, U, V;
};

YUV rgb2yuv(const std::vector<unsigned char>& rgb) {
    YUV yuv;
    yuv.Y.resize(NUM_PIXELS);
    yuv.U.resize(NUM_PIXELS);
    yuv.V.resize(NUM_PIXELS);

    for (int i = 0; i < NUM_PIXELS; ++i) {
        double r = rgb[3*i];
        double g = rgb[3*i+1];
        double b = rgb[3*i+2];

        double yVal = 0.299*r + 0.587*g + 0.114*b;
        double uVal = 0.492*(b - yVal);
        double vVal = 0.877*(r - yVal);

        yuv.Y[i] = static_cast<unsigned char>(std::clamp(yVal, 0.0, 255.0));
        yuv.U[i] = static_cast<unsigned char>(std::clamp(uVal + 128.0, 0.0, 255.0));
        yuv.V[i] = static_cast<unsigned char>(std::clamp(vVal + 128.0, 0.0, 255.0));
    }
    return yuv;
}

std::vector<unsigned char> yuv2rgb(const YUV& yuv) {
    std::vector<unsigned char> rgb(NUM_PIXELS * 3);

    for (int i = 0; i < NUM_PIXELS; ++i) {
        double y = yuv.Y[i];
        double u = (double)yuv.U[i] - 128.0;
        double v = (double)yuv.V[i] - 128.0;

        double r = y + 1.140*v;
        double g = y - 0.395*u - 0.581*v;
        double b = y + 2.032*u;

        rgb[3*i]     = static_cast<unsigned char>(std::clamp(r, 0.0, 255.0));
        rgb[3*i + 1] = static_cast<unsigned char>(std::clamp(g, 0.0, 255.0));
        rgb[3*i + 2] = static_cast<unsigned char>(std::clamp(b, 0.0, 255.0));
    }
    return rgb;
}

std::vector<unsigned char> applyMethodA(const std::vector<unsigned char>& channel) {
    std::vector<int> hist(256, 0);
    for (unsigned char val : channel) hist[val]++;

    std::vector<int> cdf(256, 0);
    cdf[0] = hist[0];
    for (int i = 1; i < 256; ++i) cdf[i] = cdf[i-1] + hist[i];

    std::vector<unsigned char> output(NUM_PIXELS);
    for (int i = 0; i < NUM_PIXELS; ++i) {
        output[i] = static_cast<unsigned char>(255.0 * cdf[channel[i]] / NUM_PIXELS);
    }
    return output;
}

struct PixelInfo { unsigned char val; int idx; };
std::vector<unsigned char> applyMethodB(const std::vector<unsigned char>& channel) {
    std::vector<PixelInfo> pixels(NUM_PIXELS);
    for (int i = 0; i < NUM_PIXELS; ++i) pixels[i] = { channel[i], i };

    std::sort(pixels.begin(), pixels.end(), [](const PixelInfo& a, const PixelInfo& b) {
        return a.val < b.val;
    });

    std::vector<unsigned char> output(NUM_PIXELS);
    for (int i = 0; i < NUM_PIXELS; ++i) {
        int newVal = (i * 255) / NUM_PIXELS;
        output[pixels[i].idx] = static_cast<unsigned char>(newVal);
    }
    return output;
}

std::vector<unsigned char> applyCLAHE(const std::vector<unsigned char>& channel) {
    cv::Mat src(HEIGHT, WIDTH, CV_8UC1, const_cast<unsigned char*>(channel.data()));
    cv::Mat dst;

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(4.0, cv::Size(8, 8));
    clahe->apply(src, dst);

    std::vector<unsigned char> output(dst.begin<unsigned char>(), dst.end<unsigned char>());
    return output;
}

int main() {
    std::string filename = "towers.raw";
    std::vector<unsigned char> rgbImg = readRaw(filename);

    YUV imgYUV = rgb2yuv(rgbImg);
    
    std::vector<unsigned char> Y_MethodA = applyMethodA(imgYUV.Y);
    std::vector<unsigned char> Y_MethodB = applyMethodB(imgYUV.Y);
    std::vector<unsigned char> Y_CLAHE   = applyCLAHE(imgYUV.Y);

    YUV outA = imgYUV;
    outA.Y = Y_MethodA;
    std::vector<unsigned char> rgbA = yuv2rgb(outA);
    writeRaw("towers_methodA.raw", rgbA);

    YUV outB = imgYUV;
    outB.Y = Y_MethodB;
    std::vector<unsigned char> rgbB = yuv2rgb(outB);
    writeRaw("towers_methodB.raw", rgbB);

    YUV outC = imgYUV;
    outC.Y = Y_CLAHE;
    std::vector<unsigned char> rgbC = yuv2rgb(outC);
    writeRaw("towers_clahe.raw", rgbC);

    return 0;
}
