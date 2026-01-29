#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <iomanip>

using namespace std;

const int WIDTH = 768;
const int HEIGHT = 512;
const int CHANNELS = 3;
const int TOTAL_PIXELS = WIDTH * HEIGHT;
const int TOTAL_BYTES = TOTAL_PIXELS * CHANNELS;

int main() {
    vector<unsigned char> imgData(TOTAL_BYTES);
    ifstream inFile("sea.raw", ios::binary);
    inFile.read(reinterpret_cast<char*>(imgData.data()), TOTAL_BYTES);
    inFile.close();

    double sumR = 0, sumG = 0, sumB = 0;
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        sumR += imgData[i];
        sumG += imgData[i + 1];
        sumB += imgData[i + 2];
    }

    double muR = sumR / TOTAL_PIXELS;
    double muG = sumG / TOTAL_PIXELS;
    double muB = sumB / TOTAL_PIXELS;
    double mu = (muR + muG + muB) / 3.0;

    double alphaR = mu / muR;
    double alphaG = mu / muG;
    double alphaB = mu / muB;

    vector<unsigned char> outData(TOTAL_BYTES);
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        outData[i]     = static_cast<unsigned char>(min(255.0, imgData[i] * alphaR));
        outData[i + 1] = static_cast<unsigned char>(min(255.0, imgData[i + 1] * alphaG));
        outData[i + 2] = static_cast<unsigned char>(min(255.0, imgData[i + 2] * alphaB));
    }

    double sumR_after = 0, sumG_after = 0, sumB_after = 0;
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        sumR_after += outData[i];
        sumG_after += outData[i + 1];
        sumB_after += outData[i + 2];
    }

    double muR_after = sumR_after / TOTAL_PIXELS;
    double muG_after = sumG_after / TOTAL_PIXELS;
    double muB_after = sumB_after / TOTAL_PIXELS;

    cout << fixed << setprecision(4);
    cout << "Means Before (R, G, B): " << muR << ", " << muG << ", " << muB << endl;
    cout << "Target Mean (Global mu): " << mu << endl;
    cout << "Means After  (R, G, B): " << muR_after << ", " << muG_after << ", " << muB_after << endl;

    ofstream outFile("sea_awb.raw", ios::binary);
    outFile.write(reinterpret_cast<char*>(outData.data()), TOTAL_BYTES);
    outFile.close();

    return 0;
}