#include <iostream>
#include <Windows.h>
#include <sstream>
#include <chrono>
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



//	for (int j = 0; j < 10; ++j) {
		auto start = std::chrono::high_resolution_clock::now();

		for (int i = 0; i < 100; ++i) {
			dle::applyEffects(pImageData, { 512, 512 }, dle::ColorOverlay(), dle::Shadow());
		}

		auto end = std::chrono::high_resolution_clock::now();
		auto ellapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

		std::cout << "Time: " << ellapsed.count() << std::endl;
//	}



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
