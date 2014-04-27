#include <iostream>
#include "dle.h"
#include <Windows.h>
#include <sstream>

int main() 
{
	// Load the image
	FILE* pFic;
	fopen_s(&pFic, "img.raw", "rb");
	unsigned char* pImageData = new unsigned char[512 * 512 * 4];
	fread(pImageData, 1, 512 * 512 * 4, pFic);
	fclose(pFic);



	dle::Size imageSize{ 512, 512 };

	dle::applyLayers(pImageData, imageSize,
		dle::Layer(pImageData, imageSize, dle::kBlendMode_Normal,
			dle::Outline(),
			dle::Shadow()),
		dle::Layer(pImageData, imageSize, dle::kBlendMode_Multiply,
			dle::ColorOverlay(),
			dle::InnerGlow()));



	// Save image
	fopen_s(&pFic, "output.raw", "wb");
	fwrite(pImageData, 1, 512 * 512 * 4, pFic);
	fclose(pFic);
	delete[] pImageData;

	// Pause then quit
	system("pause");
	return 0;
}
