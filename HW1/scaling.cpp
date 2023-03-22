#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

struct RGB{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    void scaling(RGB x1y1, RGB x1y2, RGB x2y1, RGB x2y2, double xx, double yy){
        double p1, p2;
        // calculate the linear interpolation of x axis
        p1 = x1y1.b + xx * (x2y1.b - x1y1.b);
        p2 = x1y2.b + xx * (x2y2.b - x1y2.b);
        // calculate the bilinear interpolation
        b = p1 + yy * (p2 - p1);

        p1 = x1y1.g + xx * (x2y1.g - x1y1.g);
        p2 = x1y2.g + xx * (x2y2.g - x1y2.g);
        g = p1 + yy * (p2 - p1);

        p1 = x1y1.r + xx * (x2y1.r - x1y1.r);
        p2 = x1y2.r + xx * (x2y2.r - x1y2.r);
        r = p1 + yy * (p2 - p1);
    }
};

struct RGBA{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
    void scaling(RGBA x1y1, RGBA x1y2, RGBA x2y1, RGBA x2y2, double xx, double yy){
        double p1, p2;
        p1 = x1y1.b + xx * (x2y1.b - x1y1.b);
        p2 = x1y2.b + xx * (x2y2.b - x1y2.b);
        b = p1 + yy * (p2 - p1);

        p1 = x1y1.g + xx * (x2y1.g - x1y1.g);
        p2 = x1y2.g + xx * (x2y2.g - x1y2.g);
        g = p1 + yy * (p2 - p1);

        p1 = x1y1.r + xx * (x2y1.r - x1y1.r);
        p2 = x1y2.r + xx * (x2y2.r - x1y2.r);
        r = p1 + yy * (p2 - p1);

        p1 = x1y1.a + xx * (x2y1.a - x1y1.a);
        p2 = x1y2.a + xx * (x2y2.a - x1y2.a);
        a = p1 + yy * (p2 - p1);
    }
};

struct BmpFileHeader{
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
};
struct BmpInfoHeader{
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

void process(string in_file)
{
    string index = {in_file[in_file.length()-5]};

    uint16_t type;
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    fstream fin, fout1, fout2;
    
    // open input and output file
    fin.open(in_file, ios_base::binary | ios::in);
    fout1.open("output" + index + "_up.bmp", ios_base::binary | ios::out);
    fout2.open("output" + index + "_down.bmp", ios_base::binary | ios::out);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));
    // copy bits between header and pixel info
    int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
    char *value = new char[skip_bits];
    fin.read(value, skip_bits);

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;
    int image_size = infoHeader.biSizeImage;
    

    if(type == 0x4D42 && infoHeader.biBitCount > 0)   // check the header info
    {
        int size = height * weight;
        int channel = infoHeader.biBitCount / 8;
        double rate = 1.5;
        // header of up scaling
        int up_height = height * rate, up_weight = weight * rate;
        int up_size = up_height * up_weight;
        // calculate the height, weight and size after up scaling
        infoHeader.biHeight = up_height;
        infoHeader.biWidth = up_weight;
        fileHeader.bfSize = image_size + channel * (up_size - size);
        fout1.write((char*)&type, sizeof(type));
        fout1.write((char*)&fileHeader, sizeof(fileHeader));
        fout1.write((char*)&infoHeader, sizeof(infoHeader));
        fout1.write(value, skip_bits);

        // header of down scaling
        int down_height = height / rate, down_weight = weight / rate;
        int down_size = down_height * down_weight;
        // calculate the height, weight and size after down scaling
        infoHeader.biHeight = down_height;
        infoHeader.biWidth = down_weight;
        fileHeader.bfSize = image_size + channel * (down_size - size);
        fout2.write((char*)&type, sizeof(type));
        fout2.write((char*)&fileHeader, sizeof(fileHeader));
        fout2.write((char*)&infoHeader, sizeof(infoHeader));
        fout2.write(value, skip_bits);
        delete [] value;

        if(infoHeader.biBitCount == 32){    // 4 bytes per pixel
            // read img
            RGBA **img_in = new RGBA*[height];
            for(int i=0; i<height; i++){
                img_in[i] = new RGBA[weight];
                fin.read((char*)img_in[i], weight * channel);
            }

            // up scaling -----------------------------------------------------
            RGBA *img_out_up = new RGBA[up_weight];
            for (int i=0; i<up_height; i++){
                for (int j=0 ;j<up_weight; j++){
                    double x = i / rate, y = j / rate;
                    // calculate the point correspond to the original photo
                    int x1 = (int)x, x2 = (int)(x+1), y1 = (int)y, y2 = (int)(y+1);
                    // avoid the index out of range
                    x2 == height ? x2 -= 1 : x2 = x2;
                    y2 == weight ? y2 -= 1 : y2 = y2;
                    img_out_up[j].scaling(img_in[x1][y1], img_in[x1][y2], img_in[x2][y1], img_in[x2][y2], x-x1, y-y1);
                }
                fout1.write((char*)img_out_up, up_weight * channel);
            }
            // -----------------------------------------------------------------

            // down scaling ----------------------------------------------------
            RGBA *img_out_down = new RGBA[down_weight];
            for (int i=0; i<down_height; i++){
                for (int j=0 ;j<down_weight; j++){
                    double x = i * rate, y = j * rate;
                    int x1 = (int)x, x2 = (int)(x+1), y1 = (int)y, y2 = (int)(y+1);
                    x2 == height ? x2 -= 1 : x2 = x2;
                    y2 == weight ? y2 -= 1 : y2 = y2;
                    img_out_down[j].scaling(img_in[x1][y1], img_in[x1][y2], img_in[x2][y1], img_in[x2][y2], x-x1, y-y1);
                }
                fout2.write((char*)img_out_down, down_weight * channel);
            }
            // -----------------------------------------------------------------

            for(int i = 0; i < height; i++) {
                delete [] img_in[i];
            }
            delete [] img_in;

        }else if(infoHeader.biBitCount == 24){  // 3 bytes per pixel
            // read img
            RGB **img_in = new RGB*[height];
            for(int i=0; i<height; i++){
                img_in[i] = new RGB[weight];
                fin.read((char*)img_in[i], weight * channel);
            }

            // up scaling -----------------------------------------------------
            RGB *img_out_up = new RGB[up_weight];
            for (int i=0; i<up_height; i++){
                for (int j=0 ;j<up_weight; j++){
                    double x = i / rate, y = j / rate;
                    int x1 = (int)x, x2 = (int)(x+1), y1 = (int)y, y2 = (int)(y+1);
                    x2 == height ? x2 -= 1 : x2 = x2;
                    y2 == weight ? y2 -= 1 : y2 = y2;
                    img_out_up[j].scaling(img_in[x1][y1], img_in[x1][y2], img_in[x2][y1], img_in[x2][y2], x-x1, y-y1);
                }
                fout1.write((char*)img_out_up, up_weight * channel);
            }   
            // -----------------------------------------------------------------

            // down scaling ----------------------------------------------------
            RGB *img_out_down = new RGB[down_weight];
            for (int i=0; i<down_height; i++){
                for (int j=0 ;j<down_weight; j++){
                    double x = i * rate, y = j * rate;
                    int x1 = (int)x, x2 = (int)(x+1), y1 = (int)y, y2 = (int)(y+1);
                    x2 == height ? x2 -= 1 : x2 = x2;
                    y2 == weight ? y2 -= 1 : y2 = y2;
                    img_out_down[j].scaling(img_in[x1][y1], img_in[x1][y2], img_in[x2][y1], img_in[x2][y2], x-x1, y-y1);
                }
                fout2.write((char*)img_out_down, down_weight * channel);
            }
            // -----------------------------------------------------------------
            for(int i = 0; i < height; i++) {
                delete [] img_in[i];
            }
            delete [] img_in;
        }
    }
    fin.close();
    fout1.close();
    fout2.close();
}

int main(int argc, char *argv[])
{
    for(int i=1;i<argc;i++){
        // read the argument of input file name
        string in_file = argv[i];
        // process(scaling) the input file
        process(in_file);
    }
	return 0;
}