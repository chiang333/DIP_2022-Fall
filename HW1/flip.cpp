#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct RGB{
    // 3 * 8 bits per pixel 
    unsigned char b;
    unsigned char g;
    unsigned char r;
    void copy(RGB &x){
        b=x.b;
        g=x.g;
        r=x.r;
    }
};

struct RGBA{
    // 4 * 8 bits per pixel 
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
    void copy(RGBA &x){
        b=x.b;
        g=x.g;
        r=x.r;
        a=x.a;
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
// flip
void process(string in_file, string out_file){
    uint16_t type;
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    fstream fin, fout;
    
    // open input and output file
    fin.open(in_file, ios_base::binary | ios::in);
    fout.open(out_file, ios_base::binary | ios::out);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;

    if(type == 0x4D42 && infoHeader.biBitCount > 0) // check the header info
    {
        // write the header info to output file
        fout.write((char*)&type, sizeof(type));
        fout.write((char*)&fileHeader, sizeof(fileHeader));
        fout.write((char*)&infoHeader, sizeof(infoHeader));

        // copy bits between header and pixel info
        int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
        char *value = new char[skip_bits];
        fin.read(value, skip_bits);
        fout.write(value, skip_bits);
        delete [] value;

        int size = height * weight;
        int channel = infoHeader.biBitCount / 8;
        // determine whether the file is represented by 3 or 4 bytes
        if(infoHeader.biBitCount == 32){
            RGBA img_in[weight], img_out[weight];
            for (int i=0; i<height; i++){
                // read a row into 'img_in'
                fin.read((char*)img_in, weight * channel);
                // reverse the array to flip the photo horizontally
                for (int j=0 ;j<weight; j++){
                    img_out[j].copy(img_in[weight - 1 - j]);
                }
                fout.write((char*)img_out, weight * channel);
            }
        }else if(infoHeader.biBitCount == 24){
            // same as above but with 3 channels per pixel
            RGB img_in[weight], img_out[weight];
            for (int i=0; i<height; i++){
                fin.read((char*)img_in, weight * channel);
                for (int j=0 ;j<weight; j++){
                    img_out[j].copy(img_in[weight - 1 - j]);
                }
                fout.write((char*)img_out, weight * channel);
            }
        }
    }
    fin.close();
    fout.close();
}

int main(int argc, char *argv[])
{
    for(int i=1;i<argc;i++){
        // read the argument of input file name
        string in_file = argv[i];
        string index = {in_file[in_file.length()-5]};
        // process(flip) the input file
        process(in_file, "output" + index + ".bmp");
    }
	return 0;
}