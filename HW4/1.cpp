#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <string>
#include <cmath>
#include <math.h>
#include <complex>

using namespace std;

struct RGB{
    unsigned char B;
    unsigned char G;
    unsigned char R;
};

void FFT(int N,std::complex<double>* (&x), bool inverse = false)
{
    for (int i = 1, j = 0; i < N; ++i)
    {
        for (int k = N >> 1; !((j ^= k)&k); k >>= 1);
        if (i > j) swap(x[i], x[j]);
    }

    for (int k = 2; k <= N; k <<= 1)
    {
        double theta = -2.0 * M_PI / k;
        if(inverse == true) theta = -theta;

        std::complex<double> delta_w(cos(theta), sin(theta));

        for (int j = 0; j < N; j += k)
        {
            std::complex<double> w(1, 0);
            for (int i = j; i < j + k / 2; i++)
            {
                int index1 = i;
                int index2 = i + k / 2;
                index1 = index1 % N;
                index2 = index2 % N;
                std::complex<double> a = x[index1];
                std::complex<double> b = x[index2] * w;
                x[index1] = a + b;
                x[index2] = a - b;
                w *= delta_w;
            }
        }
    }
    for (int i = 0; i < N; i++)
    {
        x[i] /= sqrt(N);
    }
}
void FFT_2d(complex<double> **(&image_in), int M, int N, bool isInverse = false){
    std::complex<double> **temp = new std::complex<double>*[M];
    for (int i = 0; i < M; i++)
    {
        temp[i] = new std::complex<double>[N];
    }
    std::complex<double> *x = new std::complex<double>[N];
    for (int i = 0; i < M; i++)
    {
        for (int j = 0; j < N; j++)
        {
            x[j] = image_in[i][j];
        }
        FFT(N, x, isInverse);
        for (int j = 0; j < N; j++)
        {
            temp[i][j] = x[j];
        }
    }
    delete[] x;
    std::complex<double> *y = new std::complex<double>[M];
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < M; j++)
        {
            y[j] = temp[j][i];
        }
        FFT(M,y, isInverse);
        for (int j = 0; j < M; j++)
        {
            image_in[j][i] = y[j];
        }
    }
    delete[] y;
    for (int i = 0; i < M; i++)
    {
        delete[] temp[i];
    }
    delete[] temp;
}

void restoration(RGB **image_in, RGB **(&image_out), int height, int weight, int n){
    int M = pow(2, ceil(log2(height))), N = pow(2, ceil(log2(weight)));
    std::complex<double> **R = new std::complex<double>*[M];
    std::complex<double> **G = new std::complex<double>*[M];
    std::complex<double> **B = new std::complex<double>*[M];
    std::complex<double> **H = new std::complex<double>*[M];
    std::complex<double> **H2 = new std::complex<double>*[M];

    for (int i = 0; i < M; i++)
    {
        R[i] = new std::complex<double>[N];
        G[i] = new std::complex<double>[N];
        B[i] = new std::complex<double>[N];
        H[i] = new std::complex<double>[N];
        H2[i] = new std::complex<double>[N];
        for(int j=0; j<N; j++){
            int ii = i > M/2 ? M-i : i;
            int jj = j > N/2 ? N-j : j;
            if(n==1){
                double t = 500;
                double gaussion_coef = exp(-double(ii*ii/(t*4) + jj*jj/t)/2.) * M / 1.33 / M_PI / t;
                H2[i][j].real(gaussion_coef);
                H2[i][j].imag(0);
            }else{
                double t = 3000;
                double gaussion_coef = exp(-double(ii*ii/t + jj*jj/t)/2.) * 4*M / M_PI / t;
                H2[i][j].real(gaussion_coef);
                H2[i][j].imag(0);
            }
            
            double xx, yy;
            if(n==1){
                xx = 30;
                yy = 42;
            }else{
                xx = -38;
                yy = -32;
            }
            ii = i;
            jj = j;
            double coef1 = M_PI * (double(ii * xx) / double(M) - double(jj * yy) / double(N));
            ii = i-M;
            double coef2 = M_PI * (double(ii * xx) / double(M) - double(jj * yy) / double(N));
            jj = j-N;
            double coef3 = M_PI * (double(ii * xx) / double(M) - double(jj * yy) / double(N));
            ii = i;
            double coef4 = M_PI * (double(ii * xx) / double(M) - double(jj * yy) / double(N));
            
            if(coef1==0 || coef2==0 || coef3==0 || coef4==0){
                H[i][j].real(1);
                H[i][j].imag(0);
            }else{
                double r = (1/coef1) * sin(coef1) * cos(-coef1) + 
                            (1/coef2) * sin(coef2) * cos(-coef2) +
                            (1/coef3) * sin(coef3) * cos(-coef3) + 
                            (1/coef4) * sin(coef4) * cos(-coef4);
                H[i][j].real(r);
                double im = (1./coef1) * sin(coef1) * sin(-coef1) + 
                            (1./coef2) * sin(coef2) * sin(-coef2) +
                            (1./coef3) * sin(coef3) * sin(-coef3) + 
                            (1./coef4) * sin(coef4) * sin(-coef4);
                H[i][j].imag(im);
            }
            if(i < height && j < weight){
                R[i][j].real(image_in[i][j].R);
                G[i][j].real(image_in[i][j].G);
                B[i][j].real(image_in[i][j].B);
            }else{
                int ii = i%height;
                int jj = j%weight;
                R[i][j].real(image_in[ii][jj].R);
                G[i][j].real(image_in[ii][jj].G);
                B[i][j].real(image_in[ii][jj].B);
            }
            R[i][j].imag(0);
            G[i][j].imag(0);
            B[i][j].imag(0);
        }
    }
    FFT_2d(R, M, N);
    FFT_2d(G, M, N);
    FFT_2d(B, M, N);
    double k;
    if(n==1){
        k = 0.018;
    }else{
        k = 0.05;
    }

    for (int i = 0; i < M; i++)
    {
        for(int j = 0; j < N; j++){
            H[i][j] *= H2[i][j];
            double hh = H[i][j].real() * H[i][j].real() + H[i][j].imag() * H[i][j].imag();
            R[i][j] = R[i][j] * hh / (k+hh) / H[i][j];
            G[i][j] = G[i][j] * hh / (k+hh) / H[i][j];
            B[i][j] = B[i][j] * hh / (k+hh) / H[i][j];
        }
    }

    FFT_2d(R, M, N, true);
    FFT_2d(G, M, N, true);
    FFT_2d(B, M, N, true);
    for (int i = 0; i < height; i++)
    {
        for(int j=0; j<weight; j++){
            double temp;
            int ii = i-20, jj = j+27;
            if(n==1){
                ii = i-20;
                jj = j+27;
                if(ii < 0) ii += M;
            }else{
                ii = i;
                jj = j;
            }
            
            temp = R[ii][jj].real();
            image_out[i][j].R = temp<=255?temp: 255;
            temp = G[ii][jj].real();
            image_out[i][j].G = temp<=255?temp: 255;
            temp = B[ii][jj].real();
            image_out[i][j].B = temp<=255?temp: 255;
        }
    }
    for (int i = 0; i < height; i++)
    {
        delete [] R[i];
        delete [] G[i];
        delete [] B[i];
        delete [] H[i];
        delete [] H2[i];
    }
    delete [] R;
    delete [] G;
    delete [] B;
    delete [] H;
    delete [] H2;
}
void evaluate(RGB **image_out, RGB **image_in_ori, int height, int weight){
    double mseR=0, mseG=0, mseB=0;
    for(int i=0;i<height;i++){
        for(int j=0;j<weight;j++){
            mseR += abs(-image_out[i][j].R+image_in_ori[i][j].R);
            mseG += abs(-image_out[i][j].G+image_in_ori[i][j].G);
            mseB += abs(-image_out[i][j].B+image_in_ori[i][j].B);
        }
    }
    mseR /= double(height*weight);
    mseG /= double(height*weight);
    mseB /= double(height*weight);
    double psnr=0;
    psnr += 10.*log10(255.*255./mseR);
    psnr += 10.*log10(255.*255./mseG);
    psnr += 10.*log10(255.*255./mseB);
    cout << psnr << endl;
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

void process(string in_file, string out_file, int n){
    uint16_t type;
    BmpFileHeader fileHeader;
    BmpInfoHeader infoHeader;
    fstream fin, fin2, fout;
    
    // open input and output file
    fin.open(in_file, ios_base::binary | ios::in);
    // fin2.open(in_file2, ios_base::binary | ios::in);
    
    // read the header of the bmp file
    fin.read((char*)&type, sizeof(type));
    fin.read((char*)&fileHeader, sizeof(BmpFileHeader));
    fin.read((char*)&infoHeader, sizeof(BmpInfoHeader));
    int skip_bits = fileHeader.bfOffBits - sizeof(type) - sizeof(fileHeader) - sizeof(infoHeader);
    char *value = new char[skip_bits];
    fin.read(value, skip_bits);

    // fin2.read((char*)&type, sizeof(type));
    // fin2.read((char*)&fileHeader, sizeof(BmpFileHeader));
    // fin2.read((char*)&infoHeader, sizeof(BmpInfoHeader));
    // fin2.read(value, skip_bits);

	int height = infoHeader.biHeight, weight = infoHeader.biWidth;
    int size = height * weight;
    int channel = infoHeader.biBitCount / 8;

    if(type == 0x4D42 && infoHeader.biBitCount > 0) // check the header info
    {   
        if(infoHeader.biBitCount == 24){
            RGB **img_in = new RGB*[height];
            // RGB **img_in2 = new RGB*[height];
            RGB **img_out = new RGB*[height];
            int read_length = weight * channel;
            // address the input format issue
            if(read_length % 4 != 0){
                read_length = ((read_length/4) + 1) * 4;
            }
            for(int i=0; i<height; i++){
                img_in[i] = new RGB[weight];
                // img_in2[i] = new RGB[weight];
                img_out[i] = new RGB[weight];
                fin.read((char*)img_in[i], read_length);
                // fin2.read((char*)img_in2[i], read_length);
            }

            fout.open(out_file, ios_base::binary | ios::out);
            // write the header info to output file
            fout.write((char*)&type, sizeof(type));
            fout.write((char*)&fileHeader, sizeof(fileHeader));
            fout.write((char*)&infoHeader, sizeof(infoHeader));
            fout.write(value, skip_bits);

            // evaluate(img_in, img_in2, height, weight);
            restoration(img_in, img_out, height, weight, n);
            // evaluate(img_out, img_in2, height, weight);
            

            for(int i=0; i<height; i++){
                fout.write((char*)img_out[i], read_length);
            }
            fout.close();
            
            for(int i = 0; i < height; i++) {
                delete [] img_in[i];
                // delete [] img_in2[i];
                delete [] img_out[i];
            }
            delete [] img_in;
            // delete [] img_in2;
            delete [] img_out;
        }
    }
    fin.close();
    
}

int main(int argc, char *argv[])
{
    // process("input1.bmp", "output1.bmp", 1);
    // process("input1.bmp", "image1_ori.bmp", "output1.bmp");
    process("input2.bmp", "output2.bmp", 2);
	return 0;
}