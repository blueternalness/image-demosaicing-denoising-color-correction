#include <iostream>
#include <vector>
#include <fstream>
#include <numeric>
#include <algorithm>

using namespace std;

// Image dimensions
const int WIDTH = 768;
const int HEIGHT = 512;
const int CHANNELS = 3;
const int TOTAL_PIXELS = WIDTH * HEIGHT;
const int TOTAL_BYTES = TOTAL_PIXELS * CHANNELS;

int main() {
    // 1. Load the raw image data
    vector<unsigned char> imgData(TOTAL_BYTES);
    ifstream inFile("sea.raw", ios::binary);
    if (!inFile) {
        cerr << "Error: Could not open input file." << endl;
        return 1;
    }
    inFile.read(reinterpret_cast<char*>(imgData.data()), TOTAL_BYTES);
    inFile.close();

    // 2. Calculate the averages of each R, G, and B channel
    double sumR = 0, sumG = 0, sumB = 0;
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        sumR += imgData[i];
        sumG += imgData[i + 1];
        sumB += imgData[i + 2];
    }

    double muR = sumR / TOTAL_PIXELS;
    double muG = sumG / TOTAL_PIXELS;
    double muB = sumB / TOTAL_PIXELS;

    cout << "Averages - R: " << muR << ", G: " << muG << ", B: " << muB << endl;

    // 3. Calculate target grayscale value (mu)
    double mu = (muR + muG + muB) / 3.0;
    cout << "Target Mean (mu): " << mu << endl;

    // 4. Calculate gain coefficients
    double alphaR = mu / muR;
    double alphaG = mu / muG;
    double alphaB = mu / muB;

    // 5. Apply gains and clip values to [0, 255]
    vector<unsigned char> outData(TOTAL_BYTES);
    for (int i = 0; i < TOTAL_BYTES; i += 3) {
        // Apply alpha and clip
        outData[i]     = static_cast<unsigned char>(min(255.0, imgData[i] * alphaR));
        outData[i + 1] = static_cast<unsigned char>(min(255.0, imgData[i + 1] * alphaG));
        outData[i + 2] = static_cast<unsigned char>(min(255.0, imgData[i + 2] * alphaB));
    }

    // 6. Save the corrected image
    ofstream outFile("sea_awb.raw", ios::binary);
    outFile.write(reinterpret_cast<char*>(outData.data()), TOTAL_BYTES);
    outFile.close();

    cout << "AWB correction complete. Saved to sea_awb.raw" << endl;

    return 0;
}
