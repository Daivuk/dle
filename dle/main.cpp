#include <iostream>
#include "dle.h"
#include <Windows.h>
#include <sstream>

int main() 
{
	// Load the image
	FILE* pFic;
	fopen_s(&pFic, "img.raw", "rb");
	unsigned char* pImage = new unsigned char[512 * 512 * 4];
	fread(pImage, 1, 512 * 512 * 4, pFic);
	fclose(pFic);

	// Create effects
	dle::ColorOverlay* pColorOverlay = dle::ColorOverlay::create({ 255, 0, 100, 255 });
	dle::Outline* pOutline = dle::Outline::create();
	dle::Shadow* pShadow = dle::Shadow::create({ 0, 0, 0, 255 }, { 2, 3 }, 1);
	dle::InnerShadow* pInnerShadow = dle::InnerShadow::create();
	dle::Glow* pGlow = dle::Glow::create();
	dle::InnerGlow* pInnerGlow = dle::InnerGlow::create();

	// Create layers
	dle::Layer* pLayer = dle::Layer::create(pImage, { 512, 512 }, { 
		pColorOverlay,
		pInnerGlow,
	//	pInnerShadow,
	//	pShadow,
		pOutline,
	//	pGlow,
	});

	// Bake
	memset(pImage, 0, 512 * 512 * 4);
	dle::bake(pImage, { 
		pLayer,
	});

	// Release resources
	pColorOverlay->release();
	pOutline->release();
	pShadow->release();
	pInnerShadow->release();
	pGlow->release();
	pInnerGlow->release();
	pLayer->release();

	// Save image
	fopen_s(&pFic, "output.raw", "wb");
	fwrite(pImage, 1, 512 * 512 * 4, pFic);
	fclose(pFic);

	// Pause then quit
	system("pause");
	return 0;
}
