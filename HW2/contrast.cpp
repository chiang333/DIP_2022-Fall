#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;

struct RGBA{
    unsigned char B;
    unsigned char G;
    unsigned char R;
    unsigned char A;
    void toYUV(struct YUV &y);
};
struct YUV{
    int Y;
    int U;
    int V;
    unsigned char A;
    void toRGB(struct RGBA &y);
};
void RGBA::toYUV(struct YUV &y){
    // RGB tp YUV (range: Y, U, V -> [0, 255])
    y.Y = 0.299 * R + 0.587 * G + 0.114 * B;
    y.U = -0.169 * R - 0.331 * G + 0.500 * B + 128;
    y.V = 0.500 * R - 0.419 * G - 0.081 * B + 128;
    y.A = A;
}
void YUV::toRGB(struct RGBA &y){
    int R, G, B;
    // YUV tp RGB (range: R, G, B -> [0, 255])
    Y = Y < 0? 0 : Y>255? 255 : Y;
    R = Y + 1.1398 * (V - 128);
    G = Y - 0.3946 * (U - 128) - 0.5806 * (V - 128);
    B = Y + 2.0321 * (U - 128);
    R < 0? y.R = 0 : R>255? y.R = 255 : y.R = R;
    G < 0? y.G = 0 : G>255? y.G = 255 : y.G = G;
    B < 0? y.B = 0 : B>255? y.B = 255 : y.B = B;
    y.A = A;
}
void enhance1(RGBA **image_in, RGBA **(&image_out), int height, int weight){
    // use gamma correction to perform image enhancement

    int size = height*weight;
    struct YUV *y = new YUV[size];
    
    // tranformd RGB to YCbCr and calculate the gamma correction using gamma = 1.5
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            image_in[i][j].toYUV(y[i*weight + j]);
            y[i*weight + j].Y = pow(y[i*weight + j].Y / 255., 1.5) * 255;
            y[i*weight + j].toRGB(image_out[i][j]);
        }
    }
}
void enhance2(RGBA **image_in, RGBA **(&image_out), int height, int weight){
    // histogram equalization

    int size = height*weight;
    struct YUV *y = new YUV[size];
    int hist[256] = {0};
    
    // tranformd RGB to YCbCr and calculate the histogram
    int max = 0, min = 300;
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            image_in[i][j].toYUV(y[i*weight + j]);
            hist[y[i*weight + j].Y] += 1;
        }
    }
    int sum = 0;
    int map[256] = {0};
    // calculate the tranform map
    for (int i = 0; i < 256; i++) {
        sum += hist[i];
        map[i] = round((((float)sum) * 255) / size);
    }
    // transform to new Y value
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            y[i*weight + j].Y = map[y[i*weight + j].Y];
            y[i*weight + j].toRGB(image_out[i][j]);
        }
    }
}

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

void process(string in_file, string out_file1, string out_file2){
    uint16_t type;
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    fstream fin, fout1, fout2;
    
    // open input and output file
    fin.open(in_file, ios_base::binary | ios::in);
    fout1.open(out_file1, ios_base::binary | ios::out);
    fout2.open(out_file2, ios_base::binary | ios::out);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;

    if(type == 0x4D42 && infoHeader.biBitCount > 0) // check the header info
    {
        // write the header info to output file
        fout1.write((char*)&type, sizeof(type));
        fout1.write((char*)&fileHeader, sizeof(fileHeader));
        fout1.write((char*)&infoHeader, sizeof(infoHeader));

        fout2.write((char*)&type, sizeof(type));
        fout2.write((char*)&fileHeader, sizeof(fileHeader));
        fout2.write((char*)&infoHeader, sizeof(infoHeader));

        // copy bits between header and pixel info
        int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
        char *value = new char[skip_bits];
        fin.read(value, skip_bits);
        fout1.write(value, skip_bits);
        fout2.write(value, skip_bits);
        delete [] value;

        int size = height * weight;
        int channel = infoHeader.biBitCount / 8;
        
        if(infoHeader.biBitCount == 32){
            RGBA **img_in = new RGBA*[height];
            RGBA **img_out1 = new RGBA*[height];
            RGBA **img_out2 = new RGBA*[height];
            for(int i=0; i<height; i++){
                img_in[i] = new RGBA[weight];
                img_out1[i] = new RGBA[weight];
                img_out2[i] = new RGBA[weight];
                fin.read((char*)img_in[i], weight * channel);
            }
            // contrast enhancement
            enhance1(img_in, img_out1, height, weight);
            enhance2(img_in, img_out2, height, weight);

            for(int i=0; i<height; i++){
                fout1.write((char*)img_out1[i], weight * channel);
                fout2.write((char*)img_out2[i], weight * channel);
            }
            
            for(int i = 0; i < height; i++) {
                delete [] img_in[i];
                delete [] img_out1[i];
                delete [] img_out2[i];
            }
            delete [] img_in;
            delete [] img_out1;
            delete [] img_out2;
        }
    }
    fin.close();
    fout1.close();
    fout2.close();
}

int main(int argc, char *argv[])
{
    process("input1.bmp", "output1_1.bmp", "output1_2.bmp");
	return 0;
}