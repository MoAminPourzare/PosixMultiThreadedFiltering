#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <pthread.h>
#include <chrono>

using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using namespace std;

#pragma pack(1)
#pragma once

#define SIZE_OF_RGB 3
#define RED 'r'
#define BLUE 'b'
#define GREEN 'g'
#define SIZE_OF_MIRROR_THREADS 12
#define SIZE_OF_IN_MIRROR_THREADS 30

#define SIZE_OF_Checkered_THREADS 16
#define SIZE_OF_IN_Checkered_THREADS 22

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
void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize) {
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


// void* mirrorFilterThread(void* arguments){
// 	long argFunc = (long) arguments;
//     if(argFunc == BLUE) {
//          for(int j = 0; j < rows; j++)
//           	reverse(bmpImage.blue[j].begin(), bmpImage.blue[j].end());
// 	}
//     else if(argFunc == GREEN) {
//         for(int j = 0; j < rows; j++)
//           	reverse(bmpImage.green[j].begin(), bmpImage.green[j].end());
// 	}
//     else if(argFunc == RED) {
//         for(int j = 0; j < rows; j++)
//           	reverse(bmpImage.red[j].begin(), bmpImage.red[j].end());
// 	}
// 	return nullptr;
// }

void* mirrorFilterThreadBlue(void* argument){
	long argFunc = (long) argument;
	int start = SIZE_OF_IN_MIRROR_THREADS * argFunc;
	int end = (SIZE_OF_IN_MIRROR_THREADS * argFunc) + SIZE_OF_IN_MIRROR_THREADS;
	for (int i = start; i < end; i++) {
		reverse(bmpImage.blue[i].begin(), bmpImage.blue[i].end());
	}
	pthread_exit(0);
}
void* mirrorFilterThreadGreen(void* argument){
	long argFunc = (long) argument;
	int start = (SIZE_OF_IN_MIRROR_THREADS * argFunc);
	int end = (SIZE_OF_IN_MIRROR_THREADS * argFunc) + SIZE_OF_IN_MIRROR_THREADS;
	for (int i = start; i < end; i++) {
		reverse(bmpImage.green[i].begin(), bmpImage.green[i].end());
	}
	pthread_exit(0);
}
void* mirrorFilterThreadRed(void* argument){
	long argFunc = (long) argument;
	int start = (SIZE_OF_IN_MIRROR_THREADS * argFunc);
	int end = (SIZE_OF_IN_MIRROR_THREADS * argFunc) + SIZE_OF_IN_MIRROR_THREADS;
	for (int i = start; i < end; i++) {
		reverse(bmpImage.red[i].begin(), bmpImage.red[i].end());
	}
	pthread_exit(0);
}
void mirrorFilter() {
	// char red = RED;
	// char blue = BLUE;
	// char green = GREEN;
	// pthread_t threads[3];
    // pthread_create(&threads[0], nullptr, &mirrorFilterThread, (void*) red);
    // pthread_create(&threads[1], nullptr, &mirrorFilterThread, (void*) green);
    // pthread_create(&threads[2], nullptr, &mirrorFilterThread, (void*) blue);
    // for (unsigned long i : threads)
    //     pthread_join(i, nullptr);
	pthread_t threadsGreen[SIZE_OF_MIRROR_THREADS];
	pthread_t threadsBlue[SIZE_OF_MIRROR_THREADS];
	pthread_t threadsRed[SIZE_OF_MIRROR_THREADS];
	for(int j = 0; j < SIZE_OF_MIRROR_THREADS; j++) {
		pthread_create(&threadsRed[j], nullptr, &mirrorFilterThreadRed, (void*) j);
		pthread_create(&threadsGreen[j], nullptr, &mirrorFilterThreadGreen, (void*) j);
		pthread_create(&threadsBlue[j], nullptr, &mirrorFilterThreadBlue, (void*) j);
	}
	for (unsigned long i : threadsGreen)
        pthread_join(i, nullptr);
	for (unsigned long i : threadsBlue)
        pthread_join(i, nullptr);
	for (unsigned long i : threadsRed)
        pthread_join(i, nullptr);
}

void* checkeredFilterThreadBlue(void* argument){
	long argFunc = (long) argument;
	argFunc = argFunc + 1;
	// cout << argFunc << endl;  
	int start = (SIZE_OF_IN_Checkered_THREADS * argFunc) - (SIZE_OF_IN_Checkered_THREADS - 1);
	int end = (SIZE_OF_IN_Checkered_THREADS * argFunc) + 1;
	// cout << start << "-->" << end << endl;  
	for (int i = start; i < end; i++) {
		for (int j = 1; j < cols - 1; j++) {
			bmpImage.blue[i][j] = (tempBmp.blue[i][j] * 1) +
			(tempBmp.blue[i][j-1] * -1) + (tempBmp.blue[i+1][j-1] * 0) +
			(tempBmp.blue[i+1][j] * 1) + (tempBmp.blue[i+1][j+1] * 2) + 
			(tempBmp.blue[i][j+1] * 1) + (tempBmp.blue[i-1][j+1] * 0) + 
			(tempBmp.blue[i-1][j] * -1) + (tempBmp.blue[i-1][j-1] * -2);		
			if (bmpImage.blue[i][j] < 0)
				bmpImage.blue[i][j] = 0;
			if (bmpImage.blue[i][j] > 255)
				bmpImage.blue[i][j] = 255;
		}
	}
	pthread_exit(0);
	return nullptr;
}
void* checkeredFilterThreadRed(void* argument){
	long argFunc = (long) argument;
	argFunc = argFunc + 1;
	// cout << argFunc << endl;  
	int start = (SIZE_OF_IN_Checkered_THREADS * argFunc) - (SIZE_OF_IN_Checkered_THREADS - 1);
	int end = (SIZE_OF_IN_Checkered_THREADS * argFunc) + 1;
	// cout << start << "-->" << end << endl;  
	for (int i = start; i < end; i++) {
		for (int j = 1; j < cols - 1; j++) {
			bmpImage.red[i][j] = (tempBmp.red[i][j] * 1) +
			(tempBmp.red[i][j-1] * -1) + (tempBmp.red[i+1][j-1] * 0) +
			(tempBmp.red[i+1][j] * 1) + (tempBmp.red[i+1][j+1] * 2) + 
			(tempBmp.red[i][j+1] * 1) + (tempBmp.red[i-1][j+1] * 0) + 
			(tempBmp.red[i-1][j] * -1) + (tempBmp.red[i-1][j-1] * -2);		
			if (bmpImage.red[i][j] < 0)
				bmpImage.red[i][j] = 0;
			if (bmpImage.red[i][j] > 255)
				bmpImage.red[i][j] = 255;
		}
	}
	pthread_exit(0);
	return nullptr;
}
void* checkeredFilterThreadGreen(void* argument){
	long argFunc = (long) argument;
	argFunc = argFunc + 1;
	// cout << argFunc << endl;  
	int start = (SIZE_OF_IN_Checkered_THREADS * argFunc) - (SIZE_OF_IN_Checkered_THREADS - 1);
	int end = (SIZE_OF_IN_Checkered_THREADS * argFunc) + 1;
	// cout << start << "-->" << end << endl;  
	for (int i = start; i < end; i++) {
		for (int j = 1; j < cols - 1; j++) {
			bmpImage.green[i][j] = (tempBmp.green[i][j] * 1) +
			(tempBmp.green[i][j-1] * -1) + (tempBmp.green[i+1][j-1] * 0) +
			(tempBmp.green[i+1][j] * 1) + (tempBmp.green[i+1][j+1] * 2) + 
			(tempBmp.green[i][j+1] * 1) + (tempBmp.green[i-1][j+1] * 0) + 
			(tempBmp.green[i-1][j] * -1) + (tempBmp.green[i-1][j-1] * -2);		
			if (bmpImage.green[i][j] < 0)
				bmpImage.green[i][j] = 0;
			if (bmpImage.green[i][j] > 255)
				bmpImage.green[i][j] = 255;
		}
	}
	pthread_exit(0);
	return nullptr;
}

// void* checkeredFilterThread(void* argument){
// 	long argFunc = (long) argument;
// 	if(argFunc == BLUE) {
// 		for (int i = 1; i < rows - 1; i++) {
// 			for (int j = 1; j < cols - 1; j++) {
// 				tempBmp.blue[i][j] = (bmpImage.blue[i][j] * 1) +
// 			 	(bmpImage.blue[i][j-1] * -1) + (bmpImage.blue[i+1][j-1] * 0) +
// 			  	(bmpImage.blue[i+1][j] * 1) + (bmpImage.blue[i+1][j+1] * 2) + 
// 			  	(bmpImage.blue[i][j+1] * 1) + (bmpImage.blue[i-1][j+1] * 0) + 
// 			  	(bmpImage.blue[i-1][j] * -1) + (bmpImage.blue[i-1][j-1] * -2);
// 				if (tempBmp.blue[i][j] < 0)
// 					tempBmp.blue[i][j] = 0;
// 				if (tempBmp.blue[i][j] > 255)
// 					tempBmp.blue[i][j] = 255;
// 			}
// 		}
// 		bmpImage.blue = tempBmp.blue;
// 	}
//     else if(argFunc == RED) {
// 		for (int i = 1; i < rows - 1; i++) {
// 			for (int j = 1; j < cols - 1; j++) {
// 				tempBmp.red[i][j] = (bmpImage.red[i][j] * 1) +
// 				(bmpImage.red[i][j-1] * -1) + (bmpImage.red[i+1][j-1] * 0) +
// 				(bmpImage.red[i+1][j] * 1) + (bmpImage.red[i+1][j+1] * 2) +
// 				(bmpImage.red[i][j+1] * 1) + (bmpImage.red[i-1][j+1] * 0) +
// 				(bmpImage.red[i-1][j] * -1) + (bmpImage.red[i-1][j-1] * -2);
// 				if (tempBmp.red[i][j] < 0)
// 					tempBmp.red[i][j] = 0;
// 				if (tempBmp.red[i][j] > 255)
// 					tempBmp.red[i][j] = 255;
// 			}
// 		}
// 		bmpImage.red = tempBmp.red;
// 	}
//     else if(argFunc == GREEN) {
// 		for (int i = 1; i < rows - 1; i++) {
// 			for (int j = 1; j < cols - 1; j++) {
// 				tempBmp.green[i][j] = (bmpImage.green[i][j] * 1) +
// 				(bmpImage.green[i][j-1] * -1) + (bmpImage.green[i+1][j-1] * 0) +
// 				(bmpImage.green[i+1][j] * 1) + (bmpImage.green[i+1][j+1] * 2) +
// 				(bmpImage.green[i][j+1] * 1) + (bmpImage.green[i-1][j+1] * 0) + 
// 				(bmpImage.green[i-1][j] * -1) + (bmpImage.green[i-1][j-1] * -2);
// 				if (tempBmp.green[i][j] < 0)
// 					tempBmp.green[i][j] = 0;
// 				if (tempBmp.green[i][j] > 255)
// 					tempBmp.green[i][j] = 255;
// 			}
// 		}
// 		bmpImage.green = tempBmp.green;
// 	}
// 	return nullptr;
// }

void checkeredFilter() {
	tempBmp = bmpImage;
	pthread_t threadsGreen[SIZE_OF_Checkered_THREADS];
	pthread_t threadsBlue[SIZE_OF_Checkered_THREADS];
	pthread_t threadsRed[SIZE_OF_Checkered_THREADS];
	for(int k = 0; k < SIZE_OF_Checkered_THREADS; k++) {
		pthread_create(&threadsGreen[k], nullptr, &checkeredFilterThreadGreen, (void*) k);
		pthread_create(&threadsBlue[k], nullptr, &checkeredFilterThreadBlue, (void*) k);
		pthread_create(&threadsRed[k], nullptr, &checkeredFilterThreadRed, (void*) k);
	}

	for (unsigned long i : threadsGreen)
        pthread_join(i, nullptr);
	for (unsigned long i : threadsBlue)
        pthread_join(i, nullptr);
	for (unsigned long i : threadsRed)
        pthread_join(i, nullptr);

	// char red = RED;
	// char blue = BLUE;
	// char green = GREEN;
	// pthread_t threads[SIZE_OF_RGB];
	// pthread_create(&threads[1], nullptr, &checkeredFilterThread, (void*) green);
	// pthread_create(&threads[0], nullptr, &checkeredFilterThread, (void*) red);
    // pthread_create(&threads[2], nullptr, &checkeredFilterThread, (void*) blue);
	// for (unsigned long i : threads)
    //     pthread_join(i, nullptr);
}

void* diamondFilterStep1(void* argument){
	long argFunc = (long) argument;
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
  	if(argFunc == RED)
    	for (int i = 0; i < midOfCols; i++) {
			bmpImage.red[counter1][i] = 255;
			counter1--;
    	} 
	else if(argFunc == GREEN)
		for (int i = 0; i < midOfCols; i++) {
			bmpImage.green[counter1][i] = 255;
			counter1--;
		}
	else if(argFunc == BLUE)
		for (int i = 0; i < midOfCols; i++) {
			bmpImage.blue[counter1][i] = 255;
			counter1--;
		} 
  	return nullptr; 
}
void* diamondFilterStep2(void* argument){
	long argFunc = (long) argument;
	int midOfRows = 0, midOfCols = 0;
	if(cols % 2 != 0)
		midOfCols = (cols+1)/2;
	else
		midOfCols = cols/2;
	if(rows % 2 != 0)
		midOfRows = (rows+1)/2;
	else
		midOfRows = rows/2;
	int counter2 = midOfRows;
  	if(argFunc == RED)
    	for (int i = 0; i < midOfCols; i++) {
			bmpImage.red[counter2][i] = 255;
			counter2++;
    	} 
	else if(argFunc == GREEN)
		for (int i = 0; i < midOfCols; i++) {
			bmpImage.green[counter2][i] = 255;
			counter2++;
		}
	else if(argFunc == BLUE)
		for (int i = 0; i < midOfCols; i++) {
			bmpImage.blue[counter2][i] = 255;
			counter2++;
		} 
  	return nullptr; 
}
void* diamondFilterStep3(void* argument){
	long argFunc = (long) argument;
	int midOfRows = 0, midOfCols = 0;
	if(cols % 2 != 0)
		midOfCols = (cols+1)/2;
	else
		midOfCols = cols/2;
	if(rows % 2 != 0)
		midOfRows = (rows+1)/2;
	else
		midOfRows = rows/2;
	int counter3 = midOfRows;
  	if(argFunc == RED)
    	for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.red[counter3][i] = 255;
			counter3--;
    	} 
	else if(argFunc == GREEN)
		for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.green[counter3][i] = 255;
			counter3--;
		} 
	else if(argFunc == BLUE)
		for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.blue[counter3][i] = 255;
			counter3--;
		} 
  	return nullptr; 
}
void* diamondFilterStep4(void* argument){
	long argFunc = (long) argument;
	int midOfRows = 0, midOfCols = 0;
	if(cols % 2 != 0)
		midOfCols = (cols+1)/2;
	else
		midOfCols = cols/2;
	if(rows % 2 != 0)
		midOfRows = (rows+1)/2;
	else
		midOfRows = rows/2;
	int counter4 = midOfRows;
  	if(argFunc == RED)
    	for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.red[counter4][i] = 255;
			counter4++;
    	} 
	else if(argFunc == GREEN)
		for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.green[counter4][i] = 255;
			counter4++;
		} 
	else if(argFunc == BLUE)
		for (int i = cols - 1; i > midOfCols; i--) {
			bmpImage.blue[counter4][i] = 255;
			counter4++;
		} 
  	return nullptr; 
}
void diamondFilter() {
	// char red = RED;
	// char blue = BLUE;
	// char green = GREEN;
	// pthread_t threads[SIZE_OF_RGB * 4];
	// pthread_create(&threads[1], nullptr, &diamondFilterStep1, (void*) green);
	// pthread_create(&threads[0], nullptr, &diamondFilterStep1, (void*) red);
    // pthread_create(&threads[2], nullptr, &diamondFilterStep1, (void*) blue);
	// pthread_create(&threads[4], nullptr, &diamondFilterStep2, (void*) green);
    // pthread_create(&threads[3], nullptr, &diamondFilterStep2, (void*) red);
    // pthread_create(&threads[5], nullptr, &diamondFilterStep2, (void*) blue);

	// pthread_create(&threads[6], nullptr, &diamondFilterStep3, (void*) green);
	// pthread_create(&threads[7], nullptr, &diamondFilterStep3, (void*) red);
    // pthread_create(&threads[8], nullptr, &diamondFilterStep3, (void*) blue);
	// pthread_create(&threads[9], nullptr, &diamondFilterStep4, (void*) green);
    // pthread_create(&threads[10], nullptr, &diamondFilterStep4, (void*) red);
    // pthread_create(&threads[11], nullptr, &diamondFilterStep4, (void*) blue);
	// for (unsigned long i : threads)
	// 	pthread_join(i, nullptr);
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

int main(int argc, char *argv[]) {
	char *fileBuffer;
	int bufferSize;
	char *fileName = argv[1];
  	auto begin = chrono::high_resolution_clock::now();
	if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
	{
		cout << "File read error" << endl;
		return 1;
	}

	// read input fil
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