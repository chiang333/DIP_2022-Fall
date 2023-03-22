#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <algorithm>
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
void enhance1(RGB **image_in, RGB **(&image_out), int height, int weight){
    // use gaussian smoothing
    int size = height*weight;
    struct YUV *y = new YUV[size];
    struct YUV t;

    // tranformd RGB to YCbCr
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            image_in[i][j].toYUV(y[i*weight + j]);
        }
    }
    // gaussian mask
    int mask[5][5] = {{1, 4, 7, 4, 1}, {4, 16, 26, 16, 4}, {7, 26, 41, 26, 7},  {4, 16, 26, 16, 4}, {1, 4, 7, 4, 1}};
    int mask_size = 5;
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            t = y[i*weight + j];
            int sum = 0;
            int sum1 = 0;
            int sum2 = 0;
            int index=0;
            for(int p=0;p<mask_size;p++){
                for(int q=0;q<mask_size;q++){
                    int ii = i + p - mask_size/2, jj = j + q - mask_size/2;
                    ii < 0? ii = 0: ii >= height? ii = height-1: ii = ii;
                    jj < 0? jj = 0: jj >= weight? jj = weight-1: jj = jj;
                    sum += y[ii*weight+jj].Y * mask[p][q];
                    sum1 += y[ii*weight+jj].U * mask[p][q];
                    sum2 += y[ii*weight+jj].V * mask[p][q];
                }
            }
            sum/=273;
            sum < 16? t.Y = 16 : sum>235? t.Y = 235 : t.Y = sum;
            sum1/=273;
            sum1 < 16? t.U = 16 : sum1>239? t.U = 239 : t.U = sum1;
            sum2/=273;
            sum2 < 16? t.V = 16 : sum2>239? t.V = 239 : t.V = sum2;
            t.toRGB(image_out[i][j]);
        }
    }
}
void enhance2(RGB **image_in, RGB **(&image_out), int height, int weight){
    // use median filter
    int size = height*weight;
    struct YUV *y = new YUV[size];
    struct YUV t;

    // tranformd RGB to YCbCr
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            image_in[i][j].toYUV(y[i*weight + j]);
        }
    }
    int mask_size = 3;
    vector<int> v;
    vector<int> position;
    for(int i=0;i<mask_size*mask_size;i++){
        v.push_back(0);
        position.push_back(i);
    }
    // median filter
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            // find the median value in the 3x3 kernal
            int index=0;
            for(int p=0;p<mask_size;p++){
                for(int q=0;q<mask_size;q++){
                    int ii = i + p - mask_size/2, jj = j + q - mask_size/2;
                    ii < 0? ii = 0: ii >= height? ii = height-1: ii = ii;
                    jj < 0? jj = 0: jj >= weight? jj = weight-1: jj = jj;
                    v[index] = y[ii*weight+jj].Y;
                    position[index] = index;
                    index++;
                }
            }
            sort( position.begin(), position.end(), [&](int i,int j){return v[i]<v[j];} );
            int p = position[mask_size*mask_size/2];
            int ii = i + p/mask_size - mask_size/2, jj = j + (p%mask_size) - mask_size/2;
            ii < 0? ii = 0: ii >= height? ii = height-1: ii = ii;
            jj < 0? jj = 0: jj >= weight? jj = weight-1: jj = jj;
            t = y[ii*weight+jj];
            t.toRGB(image_out[i][j]);
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
        
        if(infoHeader.biBitCount == 24){
            RGB **img_in = new RGB*[height];
            RGB **img_out1 = new RGB*[height];
            RGB **img_out2 = new RGB*[height];
            // read pixel value
            for(int i=0; i<height; i++){
                img_in[i] = new RGB[weight];
                img_out1[i] = new RGB[weight];
                img_out2[i] = new RGB[weight];
                fin.read((char*)img_in[i], weight * channel);
            }
            enhance1(img_in, img_out1, height, weight);
            enhance2(img_in, img_out2, height, weight);

            // output pixel value
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
    process("input3.bmp", "output3_1.bmp", "output3_2.bmp");
	return 0;
}