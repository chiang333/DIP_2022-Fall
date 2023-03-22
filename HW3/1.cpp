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
};

void maxRGB(RGB **image_in, RGB **(&image_out), int height, int weight){
    int size = height*weight;
    unsigned int R_max = 0, G_max = 0, B_max = 0;
    double R_sum = 0, G_sum = 0, B_sum = 0;
    
    // calculate max value of R, G, B saperately
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            R_max = R_max>image_in[i][j].R ? R_max : image_in[i][j].R;
            G_max = G_max>image_in[i][j].G ? G_max : image_in[i][j].G;
            B_max = B_max>image_in[i][j].B ? B_max : image_in[i][j].B;
        }
    }
    // calculate max value of all R, G, B
    unsigned int max = R_max;
    max = G_max>max ? G_max : max;
    max = B_max>max ? B_max : max;
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            // max RGB
            image_out[i][j].R = double(image_in[i][j].R) / R_max * max;
            image_out[i][j].G = double(image_in[i][j].G) / G_max * max;
            image_out[i][j].B = double(image_in[i][j].B) / B_max * max;
        }
    }
}
void greyWorld(RGB **image_in, RGB **(&image_out), int height, int weight){
    // Grey world method
    int size = height*weight;
    double R_sum = 0, G_sum = 0, B_sum = 0;
    
    // calculate average of R, G, B saperately
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            R_sum += image_in[i][j].R;
            G_sum += image_in[i][j].G;
            B_sum += image_in[i][j].B;
        }
    }
    // calculate total average value and the coefficient of RGB
    double R_co = (R_sum + G_sum + B_sum) / 3. / R_sum;
    double G_co = (R_sum + G_sum + B_sum) / 3. / G_sum;
    double B_co = (R_sum + G_sum + B_sum) / 3. / B_sum;
    double R, G, B;
    for(int i=0; i < height; i++){
        for(int j=0; j < weight; j++){
            R = double(image_in[i][j].R) * R_co;
            G = double(image_in[i][j].G) * G_co;
            B = double(image_in[i][j].B) * B_co;
            image_out[i][j].R = R > 255? 255 : R;
            image_out[i][j].G = G > 255? 255 : G;
            image_out[i][j].B = B > 255? 255 : B;
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
            // address the input format issue
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
                case 'm':
                    maxRGB(img_in, img_out, height, weight);
                    break;
                case 'g':
                    greyWorld(img_in, img_out, height, weight);
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
    process("input1.bmp", "output1_1.bmp", 'm');
    process("input2.bmp", "output2_1.bmp", 'g');
    process("input3.bmp", "output3_1.bmp", 'g');
    process("input4.bmp", "output4_1.bmp", 'g');
	return 0;
}