#include <iostream>
#include <vector>
#include <fstream>
#include <string>

const int WIDTH = 512;
const int HEIGHT = 768;

unsigned char getPixel(const std::vector<unsigned char>& img, int y, int x, int w, int h) {
    if (y < 0) y = 0;
    if (y >= h) y = h - 1;
    if (x < 0) x = 0;
    if (x >= w) x = w - 1;
    return img[y * w + x];
}

int main() {
    std::string inputFilename = "sailboats_cfa.raw";
    std::string outputFilename = "sailboats_demosaiced.raw";

    std::ifstream inFile(inputFilename, std::ios::binary);

    std::vector<unsigned char> bayerImg(std::istreambuf_iterator<char>(inFile), {});
    inFile.close();

    std::vector<unsigned char> rgbImg(WIDTH * HEIGHT * 3);
    
    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            float red = 0, green = 0, blue = 0;
            unsigned char currentVal = getPixel(bayerImg, y, x, WIDTH, HEIGHT);

            if (y % 2 == 0) {
                if (x % 2 == 0) { 
                    green = currentVal;
                    red = (getPixel(bayerImg, y, x - 1, WIDTH, HEIGHT) + getPixel(bayerImg, y, x + 1, WIDTH, HEIGHT)) / 2.0f;
                    blue = (getPixel(bayerImg, y - 1, x, WIDTH, HEIGHT) + getPixel(bayerImg, y + 1, x, WIDTH, HEIGHT)) / 2.0f;
                } else { 
                    red = currentVal;
                    green = (getPixel(bayerImg, y - 1, x, WIDTH, HEIGHT) + 
                             getPixel(bayerImg, y + 1, x, WIDTH, HEIGHT) +
                             getPixel(bayerImg, y, x - 1, WIDTH, HEIGHT) + 
                             getPixel(bayerImg, y, x + 1, WIDTH, HEIGHT)) / 4.0f;
                    blue = (getPixel(bayerImg, y - 1, x - 1, WIDTH, HEIGHT) + 
                            getPixel(bayerImg, y - 1, x + 1, WIDTH, HEIGHT) + 
                            getPixel(bayerImg, y + 1, x - 1, WIDTH, HEIGHT) + 
                            getPixel(bayerImg, y + 1, x + 1, WIDTH, HEIGHT)) / 4.0f;
                }
            } else {
                if (x % 2 == 0) {

                    blue = currentVal;
                    green = (getPixel(bayerImg, y - 1, x, WIDTH, HEIGHT) + 
                             getPixel(bayerImg, y + 1, x, WIDTH, HEIGHT) +
                             getPixel(bayerImg, y, x - 1, WIDTH, HEIGHT) + 
                             getPixel(bayerImg, y, x + 1, WIDTH, HEIGHT)) / 4.0f;
                    red = (getPixel(bayerImg, y - 1, x - 1, WIDTH, HEIGHT) + 
                           getPixel(bayerImg, y - 1, x + 1, WIDTH, HEIGHT) + 
                           getPixel(bayerImg, y + 1, x - 1, WIDTH, HEIGHT) + 
                           getPixel(bayerImg, y + 1, x + 1, WIDTH, HEIGHT)) / 4.0f;
                } else {
                    green = currentVal;
                    blue = (getPixel(bayerImg, y, x - 1, WIDTH, HEIGHT) + getPixel(bayerImg, y, x + 1, WIDTH, HEIGHT)) / 2.0f;
                    red = (getPixel(bayerImg, y - 1, x, WIDTH, HEIGHT) + getPixel(bayerImg, y + 1, x, WIDTH, HEIGHT)) / 2.0f;
                }
            }

            int index = (y * WIDTH + x) * 3;
            rgbImg[index]     = static_cast<unsigned char>(red);
            rgbImg[index + 1] = static_cast<unsigned char>(green);
            rgbImg[index + 2] = static_cast<unsigned char>(blue);
        }
    }

    std::ofstream outFile(outputFilename, std::ios::binary);

    outFile.write(reinterpret_cast<char*>(rgbImg.data()), rgbImg.size());
    outFile.close();
    
    return 0;
}