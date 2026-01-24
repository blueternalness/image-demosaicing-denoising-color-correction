#include <iostream>
#include <vector>
#include <fstream>
#include <string>

const int WIDTH = 512;
const int HEIGHT = 768;

unsigned char getVal(const std::vector<unsigned char>& img, int r, int c, int w, int h) {
    if (r < 0) r = 0;
    if (r >= h) r = h - 1;
    if (c < 0) c = 0;
    if (c >= w) c = w - 1;
    return img[r * w + c];
}

int main() {
    std::string inputFilename = "sailboats_cfa.raw";
    std::string outputFilename = "sailboats_demosaiced.raw";

    std::ifstream inFile(inputFilename, std::ios::binary);
    if (!inFile) {
        std::cerr << "Error: Could not open " << inputFilename << std::endl;
        return 1;
    }

    std::vector<unsigned char> bayerImg(std::istreambuf_iterator<char>(inFile), {});
    inFile.close();

    if (bayerImg.size() != WIDTH * HEIGHT) {
        std::cerr << "Error: File size (" << bayerImg.size() 
                  << ") does not match specified dimensions " 
                  << WIDTH << "x" << HEIGHT << " = " << (WIDTH*HEIGHT) << std::endl;
        return 1;
    }

    std::vector<unsigned char> rgbImg(WIDTH * HEIGHT * 3);
    
    for (int r = 0; r < HEIGHT; ++r) {
        for (int c = 0; c < WIDTH; ++c) {
            float red = 0, green = 0, blue = 0;
            unsigned char currentVal = getVal(bayerImg, r, c, WIDTH, HEIGHT);

            if (r % 2 == 0) {
                if (c % 2 == 0) { 
                    green = currentVal;
                    red = (getVal(bayerImg, r, c - 1, WIDTH, HEIGHT) + getVal(bayerImg, r, c + 1, WIDTH, HEIGHT)) / 2.0f;
                    blue = (getVal(bayerImg, r - 1, c, WIDTH, HEIGHT) + getVal(bayerImg, r + 1, c, WIDTH, HEIGHT)) / 2.0f;
                } else { 
                    red = currentVal;
                    green = (getVal(bayerImg, r - 1, c, WIDTH, HEIGHT) + 
                             getVal(bayerImg, r + 1, c, WIDTH, HEIGHT) +
                             getVal(bayerImg, r, c - 1, WIDTH, HEIGHT) + 
                             getVal(bayerImg, r, c + 1, WIDTH, HEIGHT)) / 4.0f;
                    blue = (getVal(bayerImg, r - 1, c - 1, WIDTH, HEIGHT) + 
                            getVal(bayerImg, r - 1, c + 1, WIDTH, HEIGHT) + 
                            getVal(bayerImg, r + 1, c - 1, WIDTH, HEIGHT) + 
                            getVal(bayerImg, r + 1, c + 1, WIDTH, HEIGHT)) / 4.0f;
                }
            } else {
                if (c % 2 == 0) {

                    blue = currentVal;
                    green = (getVal(bayerImg, r - 1, c, WIDTH, HEIGHT) + 
                             getVal(bayerImg, r + 1, c, WIDTH, HEIGHT) +
                             getVal(bayerImg, r, c - 1, WIDTH, HEIGHT) + 
                             getVal(bayerImg, r, c + 1, WIDTH, HEIGHT)) / 4.0f;
                    red = (getVal(bayerImg, r - 1, c - 1, WIDTH, HEIGHT) + 
                           getVal(bayerImg, r - 1, c + 1, WIDTH, HEIGHT) + 
                           getVal(bayerImg, r + 1, c - 1, WIDTH, HEIGHT) + 
                           getVal(bayerImg, r + 1, c + 1, WIDTH, HEIGHT)) / 4.0f;
                } else {
                    green = currentVal;
                    blue = (getVal(bayerImg, r, c - 1, WIDTH, HEIGHT) + getVal(bayerImg, r, c + 1, WIDTH, HEIGHT)) / 2.0f;
                    red = (getVal(bayerImg, r - 1, c, WIDTH, HEIGHT) + getVal(bayerImg, r + 1, c, WIDTH, HEIGHT)) / 2.0f;
                }
            }

            int index = (r * WIDTH + c) * 3;
            rgbImg[index]     = static_cast<unsigned char>(red);
            rgbImg[index + 1] = static_cast<unsigned char>(green);
            rgbImg[index + 2] = static_cast<unsigned char>(blue);
        }
    }

    std::ofstream outFile(outputFilename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not write to " << outputFilename << std::endl;
        return 1;
    }

    outFile.write(reinterpret_cast<char*>(rgbImg.data()), rgbImg.size());
    outFile.close();

    std::cout << "Success! Saved to " << outputFilename << std::endl;
    std::cout << "Output Size: " << rgbImg.size() << " bytes." << std::endl;
    std::cout << "Resolution: " << WIDTH << " x " << HEIGHT << std::endl;
    
    return 0;
}