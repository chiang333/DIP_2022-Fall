#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct RGB{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    void resolution(RGB &input, int bits_number){
        // wipe out the last bits of each channels
        b = input.b >> (bits_number) << (bits_number);
        g = input.g >> (bits_number) << (bits_number);
        r = input.r >> (bits_number) << (bits_number);
    }
};

struct RGBA{
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
    void resolution(RGBA &input, int bits_number){
        // wipe out the last bits of each channels
        b = input.b >> (bits_number) << (bits_number);
        g = input.g >> (bits_number) << (bits_number);
        r = input.r >> (bits_number) << (bits_number);
        a = input.a >> (bits_number) << (bits_number);
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
    fstream fin, fout1, fout2, fout3, fout;
    
    // open input file
    fin.open(in_file, ios_base::binary | ios::in);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));
    // copy bits between header and pixel info
    int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
    char *value = new char[skip_bits];
    fin.read(value, skip_bits);

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;

    if(type == 0x4D42 && infoHeader.biBitCount > 0)  // check the header info
    {
        int size = height * weight;
        int channel = infoHeader.biBitCount / 8;
        if(infoHeader.biBitCount == 32){    // 4 bytes per pixel
            // read img
            RGBA **img_in = new RGBA*[height];
            for(int i=0; i<height; i++){
                img_in[i] = new RGBA[weight];
                fin.read((char*)img_in[i], weight * channel);
            }
            // process the resolution with 6, 4, 2 bits
            for(int k=1; k<=3; k++){
                fout.open("output" + index + "_" + to_string(k) + ".bmp", ios_base::binary | ios::out);
                // output the file headers
                fout.write((char*)&type, sizeof(type));
                fout.write((char*)&fileHeader, sizeof(fileHeader));
                fout.write((char*)&infoHeader, sizeof(infoHeader));
                fout.write(value, skip_bits);
                
                // calculate the resolution value of each pixel and output
                RGBA img_out[weight];
                for (int i=0; i<height; i++){
                    for (int j=0 ;j<weight; j++){
                        img_out[j].resolution(img_in[i][j], k*2);
                    }
                    fout.write((char*)img_out, weight * channel);
                }
                fout.close();
            }
        }else if(infoHeader.biBitCount == 24){  // 3 bytes per pixel
            // same as above
            RGB **img_in = new RGB*[height];
            for(int i=0; i<height; i++){
                img_in[i] = new RGB[weight];
                fin.read((char*)img_in[i], weight * channel);
            }
            for(int k=1; k<=3; k++){
                fout.open("output" + index + "_" + to_string(k) + ".bmp", ios_base::binary | ios::out);

                fout.write((char*)&type, sizeof(type));
                fout.write((char*)&fileHeader, sizeof(fileHeader));
                fout.write((char*)&infoHeader, sizeof(infoHeader));
                fout.write(value, skip_bits);

                int size = height * weight;
                int channel = infoHeader.biBitCount / 8;
                
                RGB img_out[weight];
                for (int i=0; i<height; i++){
                    for (int j=0 ;j<weight; j++){
                        img_out[j].resolution(img_in[i][j], k*2);
                    }
                    fout.write((char*)img_out, weight * channel);
                }
                fout.close();
            }
        }
    }
    fin.close();
    delete [] value;
}

int main(int argc, char *argv[])
{
    for(int i=1;i<argc;i++){
        string in_file = argv[i];
        string out_file;
        process(in_file);
    }
	return 0;
}