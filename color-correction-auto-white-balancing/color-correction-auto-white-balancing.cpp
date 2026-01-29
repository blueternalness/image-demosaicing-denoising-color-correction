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
    // 1. Load the raw image
    vector<unsigned char> imgData(TOTAL_BYTES);
    ifstream inFile("sea.raw", ios::binary);
    if (!inFile) {
        cerr << "Error: Could not open sea.raw" << endl;
        return 1;
    }
    inFile.read(reinterpret_cast<char*>(imgData.data()), TOTAL_BYTES);
    inFile.close();

    // 2. Calculate means_before
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

    // 3. Calculate gain coefficients
    double alphaR = mu / muR;
    double alphaG = mu / muG;
    double alphaB = mu / muB;

    // 4. Apply AWB and Clip
    vector<unsigned char> outData(TOTAL_BYTES);
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        outData[i]     = static_cast<unsigned char>(min(255.0, imgData[i] * alphaR));
        outData[i + 1] = static_cast<unsigned char>(min(255.0, imgData[i + 1] * alphaG));
        outData[i + 2] = static_cast<unsigned char>(min(255.0, imgData[i + 2] * alphaB));
    }

    // 5. Calculate means_after
    double sumR_after = 0, sumG_after = 0, sumB_after = 0;
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        sumR_after += outData[i];
        sumG_after += outData[i + 1];
        sumB_after += outData[i + 2];
    }

    double muR_after = sumR_after / TOTAL_PIXELS;
    double muG_after = sumG_after / TOTAL_PIXELS;
    double muB_after = sumB_after / TOTAL_PIXELS;

    // 6. Print essential data for plotting
    cout << fixed << setprecision(4);
    cout << "==========================================" << endl;
    cout << "ESSENTIAL DATA FOR PLOTTING" << endl;
    cout << "==========================================" << endl;
    cout << "Means Before (R, G, B): " << muR << ", " << muG << ", " << muB << endl;
    cout << "Target Mean (Global mu): " << mu << endl;
    cout << "Means After  (R, G, B): " << muR_after << ", " << muG_after << ", " << muB_after << endl;
    cout << "==========================================" << endl;

    // Save the corrected image
    ofstream outFile("sea_awb.raw", ios::binary);
    outFile.write(reinterpret_cast<char*>(outData.data()), TOTAL_BYTES);
    outFile.close();

    return 0;
}