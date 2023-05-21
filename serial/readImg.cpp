#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>

using namespace std;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;

#pragma pack(1)
#pragma once

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct RGB {
	vector<vector<int>> blue;
    vector<vector<int>> red;
    vector<vector<int>> green;
} RGB;

int rows;
int cols;

RGB bmpImage;
RGB tempBmp;

bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
	std::ifstream file(fileName);

	if (file)
	{
		file.seekg(0, std::ios::end);
		std::streampos length = file.tellg();
		file.seekg(0, std::ios::beg);

		buffer = new char[length];
		file.read(&buffer[0], length);

		PBITMAPFILEHEADER file_header;
		PBITMAPINFOHEADER info_header;

		file_header = (PBITMAPFILEHEADER)(&buffer[0]);
		info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
		rows = info_header->biHeight;
		cols = info_header->biWidth;
		bufferSize = file_header->bfSize;
		return 1;
	}
	else
	{
		cout << "File" << fileName << " doesn't exist!" << endl;
		return 0;
	}
}

void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer) {
	int count = 1;
	int extra = cols % 4;
	unsigned char red_pixel;
	unsigned char green_pixel;
	unsigned char blue_pixel;
	vector<int> colInit(cols, 0);
	for (int i = 0; i < rows; i++) {
		count += extra;
		bmpImage.red.push_back(colInit);
		bmpImage.blue.push_back(colInit);
		bmpImage.green.push_back(colInit);
		tempBmp.red.push_back(colInit);
		tempBmp.blue.push_back(colInit);
		tempBmp.green.push_back(colInit);
		for (int j = cols - 1; j >= 0; j--) {
			for (int k = 0; k < 3; k++) {
				switch (k) {
				case 0:
					red_pixel = fileReadBuffer[end - count];
					bmpImage.red[i][j] = red_pixel;
					count++;
					break;
				case 1:
					green_pixel = fileReadBuffer[end - count];
					bmpImage.green[i][j] = green_pixel;
					count++;
					break;
				case 2:
					blue_pixel = fileReadBuffer[end - count];
					bmpImage.blue[i][j] = blue_pixel;
					count++;
					break;
				}
			}
		}
	}
}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize)
{
	std::ofstream write(nameOfFileToCreate);
	if (!write)
	{
		cout << "Failed to write " << nameOfFileToCreate << endl;
		return;
	}
	int count = 1;
	int extra = cols % 4;
	for (int i = 0; i < rows; i++)
	{
		count += extra;
		for (int j = cols - 1; j >= 0; j--) {
			for (int k = 0; k < 3; k++) {
				switch (k)
				{
				case 0:
					fileBuffer[bufferSize - count] = bmpImage.red[i][j];
					break;
				case 1:
					fileBuffer[bufferSize - count] = bmpImage.green[i][j];
					break;
				case 2:
					fileBuffer[bufferSize - count] = bmpImage.blue[i][j];
					break;
				}
				count++;
			}
		}
	}
	write.write(fileBuffer, bufferSize);
}

void mirrorFilter() {
	for(int j = 0; j < rows; j++) {
		reverse(bmpImage.green[j].begin(), bmpImage.green[j].end());
		reverse(bmpImage.blue[j].begin(), bmpImage.blue[j].end());
		reverse(bmpImage.red[j].begin(), bmpImage.red[j].end());
	}
}

void checkeredFilter(){
	// for (int i = 1; i < rows - 1; i++) {
	// 	for (int j = 1; j < cols - 1; j++) {
	// 		tempBmp.blue[i][j] = (bmpImage.blue[i][j] * 1) +
	// 		 (bmpImage.blue[i][j-1] * 1) + (bmpImage.blue[i+1][j-1] * 0) +
	// 		  (bmpImage.blue[i+1][j] * -1) + (bmpImage.blue[i+1][j+1] * -2) + 
	// 		   (bmpImage.blue[i][j+1] * -1) + (bmpImage.blue[i-1][j+1] * 0) + 
	// 		   (bmpImage.blue[i-1][j] * 1) + (bmpImage.blue[i-1][j-1] * 2);

	// 		tempBmp.red[i][j] = (bmpImage.red[i][j] * 1) +
	// 		 (bmpImage.red[i][j-1] * 1) + (bmpImage.red[i+1][j-1] * 0) +
	// 		  (bmpImage.red[i+1][j] * -1) + (bmpImage.red[i+1][j+1] * -2) +
	// 		   (bmpImage.red[i][j+1] * -1) + (bmpImage.red[i-1][j+1] * 0) +
	// 		    (bmpImage.red[i-1][j] * 1) + (bmpImage.red[i-1][j-1] * 2);

	// 		tempBmp.green[i][j] = (bmpImage.green[i][j] * 1) +
	// 		 (bmpImage.green[i][j-1] * 1) + (bmpImage.green[i+1][j-1] * 0) +
	// 		  (bmpImage.green[i+1][j] * -1) + (bmpImage.green[i+1][j+1] * -2) + 
	// 		   (bmpImage.green[i][j+1] * -1) + (bmpImage.green[i-1][j+1] * 0) + 
	// 		    (bmpImage.green[i-1][j] * 1) + (bmpImage.green[i-1][j-1] * 2);


	// 		if (tempBmp.blue[i][j] < 0)
	// 			tempBmp.blue[i][j] = 0;
	// 		if (tempBmp.blue[i][j] > 255)
	// 			tempBmp.blue[i][j] = 255;

	// 		if (tempBmp.red[i][j] < 0)
	// 			tempBmp.red[i][j] = 0;
	// 		if (tempBmp.red[i][j] > 255)
	// 			tempBmp.red[i][j] = 255;
				
	// 		if (tempBmp.green[i][j] < 0)
	// 			tempBmp.green[i][j] = 0;
	// 		if (tempBmp.green[i][j] > 255)
	// 			tempBmp.green[i][j] = 255;
	// 	}
	// }
	// bmpImage = tempBmp;
	static const int checkerboard[] = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
	static const int dX[] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
	static const int dY[] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

	tempBmp = bmpImage;


	for(int row = 1; row < rows - 1; ++row){
		for(int col = 1; col < cols - 1; ++col){
			int _red_ = 0, _green_ = 0, _blue_ = 0;
			for (int i = 0; i < 9; ++i){
				int rrr = tempBmp.red[row + dX[i]][col + dY[i]];
				int bbb = tempBmp.blue[row + dX[i]][col + dY[i]];
				int ggg = tempBmp.green[row + dX[i]][col + dY[i]];

				_red_ += checkerboard[i] * rrr;
				_green_ += checkerboard[i] * ggg;
				_blue_ += checkerboard[i] * bbb;
			}
			_red_ = std::min(255, std::max(0, _red_));
			_green_ = std::min(255, std::max(0, _green_));
			_blue_ = std::min(255, std::max(0, _blue_));
			bmpImage.red[row][col] = _red_;
			bmpImage.green[row][col] = _green_;
			bmpImage.blue[row][col] = _blue_;
		}
	}
}

void diamondFilter() {
	int midOfRows = 0, midOfCols = 0;
	if(cols % 2 != 0)
		midOfCols = (cols+1)/2;
	else
		midOfCols = cols/2;
	if(rows % 2 != 0)
		midOfRows = (rows+1)/2;
	else
		midOfRows = rows/2;
	int counter1 = midOfRows;
	int counter2 = midOfRows;
	for (int i = 0; i < midOfCols; i++) {
		bmpImage.blue[counter1][i] = 255;
		bmpImage.blue[counter2][i] = 255;
		bmpImage.green[counter1][i] = 255;
		bmpImage.green[counter2][i] = 255;
		bmpImage.red[counter1][i] = 255;
		bmpImage.red[counter2][i] = 255;
		counter1--;
		counter2++;
	}
	int counter3 = midOfRows;
	int counter4 = midOfRows;
	for (int i = cols - 1; i > midOfCols; i--) {
		bmpImage.blue[counter3][i] = 255;
		bmpImage.blue[counter4][i] = 255;
		bmpImage.green[counter3][i] = 255;
		bmpImage.green[counter4][i] = 255;
		bmpImage.red[counter3][i] = 255;
		bmpImage.red[counter4][i] = 255;
		counter3--;
		counter4++;
	}
}

int main(int argc, char *argv[])
{
	char *fileBuffer;
	int bufferSize;
	char *fileName = argv[1];
  	auto begin = chrono::high_resolution_clock::now();
	if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
	{
		cout << "File read error" << endl;
		return 1;
	}
	// read input file
	getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);

	// apply filters
	mirrorFilter();
	checkeredFilter();
	diamondFilter();

	// write output file
	writeOutBmp24(fileBuffer, "output.bmp", bufferSize);
	auto end = chrono::high_resolution_clock::now();
	cout << "Execution Time: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << endl;
	return 0;
}