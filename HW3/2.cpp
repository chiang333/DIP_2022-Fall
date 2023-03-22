#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
using namespace std;

struct RGB{
    unsigned char B;
    unsigned char G;
    unsigned char R;
    void toYUV(struct YUV &y);
};
struct YUV{
    int Y;
    int U;
    int V;
    void toRGB(struct RGB &y);
};
void RGB::toYUV(struct YUV &y){
    // RGB tp YUV (range: Y, U, V -> [16, 235], [16, 239], [16, 239])
    y.Y = 0.257 * R + 0.504 * G + 0.098 * B + 16;
    y.V = 0.439 * R - 0.368 * G - 0.071 * B + 128;
    y.U = - 0.148 * R - 0.291 * G + 0.439 * B + 128;
}
void YUV::toRGB(struct RGB &y){
    // YUV tp RGB (range: R, G, B -> [0, 255])
    int R, G, B;
    R = 1.164 * (Y - 16) + 1.596 * (V - 128);
    G = 1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128);
    B = 1.164 * (Y - 16) + 2.018 * (U - 128);
    R < 0? y.R = 0 : R>255? y.R = 255 : y.R = R;
    G < 0? y.G = 0 : G>255? y.G = 255 : y.G = G;
    B < 0? y.B = 0 : B>255? y.B = 255 : y.B = B;
}
void sharp_enhance(RGB **image_in, RGB **(&image_out), int height, int weight){
    int size = height*weight;
    struct YUV *y = new YUV[size];
    struct YUV t;
    int mask[5][5] = {{0, 0, -1, 0, 0}, {0, -1, -2, -1, 0}, {-1, -2, 16, -2, -1}, {0, -1, -2, -1, 0}, {0, 0, -1, 0, 0}};
    int mask_size = 5;
    
    // tranform RGB to Y'CbCr
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            image_in[i][j].toYUV(y[i*weight + j]);
        }
    }
    
    // image luma do convolution with sharpening filter
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            t = y[i*weight + j];
            int sum = 0;
            // convolution with the sharpen mask
            for(int p=0;p<mask_size;p++){
                for(int q=0;q<mask_size;q++){
                    int ii = i + p - mask_size/2, jj = j + q - mask_size/2;
                    ii < 0? ii = 0: ii >= height? ii = height-1: ii = ii;
                    jj < 0? jj = 0: jj >= weight? jj = weight-1: jj = jj;
                    sum += y[ii*weight+jj].Y * mask[p][q];
                }
            }
            sum *= 0.5;
            sum += y[i*weight+j].Y;
            sum < 16? t.Y = 16 : sum>235? t.Y = 235 : t.Y = sum;
            // tranform back to RGB
            t.toRGB(image_out[i][j]);
        }
    }
}
void contrast_enhance(RGB **image_in, RGB **(&image_out), int height, int weight){
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

void process(string in_file, string out_file, char method){
    uint16_t type;
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    fstream fin, fout;
    
    // open input and output file
    fin.open(in_file, ios_base::binary | ios::in);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));
    int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
    char *value = new char[skip_bits];
    fin.read(value, skip_bits);

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;
    int size = height * weight;
    int channel = infoHeader.biBitCount / 8;

    if(type == 0x4D42 && infoHeader.biBitCount > 0) // check the header info
    {   
        if(infoHeader.biBitCount == 24){
            RGB **img_in = new RGB*[height];
            RGB **img_out = new RGB*[height];
            int read_length = weight * channel;
            if(read_length % 4 != 0){
                read_length = ((read_length/4) + 1) * 4;
            }
            for(int i=0; i<height; i++){
                img_in[i] = new RGB[weight];
                img_out[i] = new RGB[weight];
                fin.read((char*)img_in[i], read_length);
            }

            fout.open(out_file, ios_base::binary | ios::out);
            // write the header info to output file
            fout.write((char*)&type, sizeof(type));
            fout.write((char*)&fileHeader, sizeof(fileHeader));
            fout.write((char*)&infoHeader, sizeof(infoHeader));
            fout.write(value, skip_bits);
            // evaluate two different method
            switch(method){
                case 's':
                    sharp_enhance(img_in, img_out, height, weight);
                    break;
                case 'c':
                    contrast_enhance(img_in, img_out, height, weight);
                    break;
                default:
                    break;
            }

            for(int i=0; i<height; i++){
                fout.write((char*)img_out[i], read_length);
            }
            fout.close();
            
            for(int i = 0; i < height; i++) {
                delete [] img_in[i];
                delete [] img_out[i];
            }
            delete [] img_in;
            delete [] img_out;
        }
    }
    fin.close();
    
}

int main(int argc, char *argv[])
{
    process("output1_1.bmp", "output1_2.bmp", 's');
    process("output2_1.bmp", "output2_2.bmp", 's');
    process("output3_1.bmp", "output3_2.bmp", 'c');
    process("output4_1.bmp", "output4_2.bmp", 'c');
	return 0;
}