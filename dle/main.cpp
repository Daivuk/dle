#include <iostream>
#include <Windows.h>
#include <sstream>
#include "dle.h"

int main() {
	// Load the image
	FILE* pFic;
	fopen_s(&pFic, "img.raw", "rb");
	unsigned char* pImageData = new unsigned char[512 * 512 * 4];
	fread(pImageData, 1, 512 * 512 * 4, pFic);
	fclose(pFic);
	fopen_s(&pFic, "img2.raw", "rb");
	unsigned char* pImage2Data = new unsigned char[512 * 512 * 4];
	fread(pImage2Data, 1, 512 * 512 * 4, pFic);
	fclose(pFic);




	dle::applyEffects(pImageData, { 512, 512, },
		dle::Outline({ 0, 0, 0, 245 }, 3),
		dle::Shadow(),
		dle::InnerShadow());




	// Save image
	fopen_s(&pFic, "output.raw", "wb");
	fwrite(pImageData, 1, 512 * 512 * 4, pFic);
	fclose(pFic);
	delete[] pImageData;
	delete[] pImage2Data;

	// Pause then quit
	system("pause");
	return 0;
}
