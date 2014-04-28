#include <stdlib.h>
#include "dle.h"
#include <assert.h>


namespace dle {

	static int g_sintable[360] = {
		0, 174, 348, 523, 697, 871, 1045, 1218, 1391, 1564, 1736, 1908, 2079, 2249, 2419, 2588, 2756, 2923, 3090, 3255,
		3420, 3583, 3746, 3907, 4067, 4226, 4383, 4539, 4694, 4848, 4999, 5150, 5299, 5446, 5591, 5735, 5877, 6018, 6156,
		6293, 6427, 6560, 6691, 6819, 6946, 7071, 7193, 7313, 7431, 7547, 7660, 7771, 7880, 7986, 8090, 8191, 8290, 8386,
		8480, 8571, 8660, 8746, 8829, 8910, 8987, 9063, 9135, 9205, 9271, 9335, 9396, 9455, 9510, 9563, 9612, 9659, 9702,
		9743, 9781, 9816, 9848, 9876, 9902, 9925, 9945, 9961, 9975, 9986, 9993, 9998, 10000, 9998, 9993, 9986, 9975,
		9961, 9945, 9925, 9902, 9876, 9848, 9816, 9781, 9743, 9702, 9659, 9612, 9563, 9510, 9455, 9396, 9335, 9271, 9205,
		9135, 9063, 8987, 8910, 8829, 8746, 8660, 8571, 8480, 8386, 8290, 8191, 8090, 7986, 7880, 7771, 7660, 7547, 7431,
		7313, 7193, 7071, 6946, 6819, 6691, 6560, 6427, 6293, 6156, 6018, 5877, 5735, 5591, 5446, 5299, 5150, 4999, 4848,
		4694, 4539, 4383, 4226, 4067, 3907, 3746, 3583, 3420, 3255, 3090, 2923, 2756, 2588, 2419, 2249, 2079, 1908, 1736,
		1564, 1391, 1218, 1045, 871, 697, 523, 348, 174, 0, -174, -348, -523, -697, -871, -1045, -1218, -1391, -1564,
		-1736, -1908, -2079, -2249, -2419, -2588, -2756, -2923, -3090, -3255, -3420, -3583, -3746, -3907, -4067, -4226,
		-4383, -4539, -4694, -4848, -5000, -5150, -5299, -5446, -5591, -5735, -5877, -6018, -6156, -6293, -6427, -6560,
		-6691, -6819, -6946, -7071, -7193, -7313, -7431, -7547, -7660, -7771, -7880, -7986, -8090, -8191, -8290, -8386,
		-8480, -8571, -8660, -8746, -8829, -8910, -8987, -9063, -9135, -9205, -9271, -9335, -9396, -9455, -9510, -9563,
		-9612, -9659, -9702, -9743, -9781, -9816, -9848, -9876, -9902, -9925, -9945, -9961, -9975, -9986, -9993, -9998,
		-10000, -9998, -9993, -9986, -9975, -9961, -9945, -9925, -9902, -9876, -9848, -9816, -9781, -9743, -9702, -9659,
		-9612, -9563, -9510, -9455, -9396, -9335, -9271, -9205, -9135, -9063, -8987, -8910, -8829, -8746, -8660, -8571,
		-8480, -8386, -8290, -8191, -8090, -7986, -7880, -7771, -7660, -7547, -7431, -7313, -7193, -7071, -6946, -6819,
		-6691, -6560, -6427, -6293, -6156, -6018, -5877, -5735, -5591, -5446, -5299, -5150, -5000, -4848, -4694, -4539,
		-4383, -4226, -4067, -3907, -3746, -3583, -3420, -3255, -3090, -2923, -2756, -2588, -2419, -2249, -2079, -1908,
		-1736, -1564, -1391, -1218, -1045, -871, -697, -523, -348, -174 };

	inline int min(int a, int b) {
		return a + (((b - a) >> 31) & (b - a));
	}

	inline int max(int a, int b) {
		return a - ((a - b) & (a - b) >> 31);
	}

	inline int clamp(int val, int in_min, int in_max) {
		return dle::min(dle::max(val, in_min), in_max);
	}

	inline int wrapAngle(int a) {
		if (a >= 0) return a % 360;
		return 360 - (-a) % 360;
	}

	inline void lerp(Color& out, const Color& a, const Color& b, const int t) {
		int invT = 255 - t;

		out.r = a.r * invT / 255 + b.r * t / 255;
		out.g = a.g * invT / 255 + b.g * t / 255;
		out.b = a.b * invT / 255 + b.b * t / 255;
		out.a = a.a * invT / 255 + b.a * t / 255;
	}

	inline void lerpPercentile(Color& out, const Color& a, const Color& b, const int t) {
		int invT = 10000 - t;

		out.r = a.r * invT / 10000 + b.r * t / 10000;
		out.g = a.g * invT / 10000 + b.g * t / 10000;
		out.b = a.b * invT / 10000 + b.b * t / 10000;
		out.a = a.a * invT / 10000 + b.a * t / 10000;
	}

	inline void lerpPreserveAlpha(Color& out, const Color& a, const Color& b, const int t) {
		int invT = 255 - t;

		out.r = a.r * invT / 255 + b.r * t / 255;
		out.g = a.g * invT / 255 + b.g * t / 255;
		out.b = a.b * invT / 255 + b.b * t / 255;
		out.a = dle::max(a.a, b.a);
	}

	void blend(Color& out, const Color& dst, const Color& src, const eBlendMode& blendmode) {
		Color tmpDst;
		Color tmpOut;
		switch (blendmode) {
		case kBlendMode_Normal:
			tmpOut.r = src.r;
			tmpOut.g = src.g;
			tmpOut.b = src.b;
			tmpOut.a = dle::min(255, dst.a + src.a);

			// Lerp final alpha composite
			lerp(out, dst, tmpOut, src.a);
			break;
		case kBlendMode_Multiply:
			tmpOut.r = dst.r * src.r / 255;
			tmpOut.g = dst.g * src.g / 255;
			tmpOut.b = dst.b * src.b / 255;
			tmpOut.a = dle::min(255, dst.a + src.a);

			// Lerp final alpha composite
			lerp(out, dst, tmpOut, src.a);
			break;
		case kBlendMode_Screen:
			// First, we want to make the dst color the same as our src color in the case
			// of the alpha being very low. Because images might be black and we end up with black
			// shadow. And we don't want that
			lerpPreserveAlpha(tmpDst, src, dst, dst.a);

			// This is screen in photoshop. It's not a real additive. But it gives a better result
			// 1 - (1 - a) * (1 - b)
			tmpOut.r = 255 - (255 - dst.r) * (255 - src.r) / 255;
			tmpOut.g = 255 - (255 - dst.g) * (255 - src.g) / 255;
			tmpOut.b = 255 - (255 - dst.b) * (255 - src.b) / 255;
			tmpOut.a = dle::min(255, dst.a + src.a);

			// Lerp final alpha composite
			lerp(out, tmpDst, tmpOut, src.a);
			break;
		}
	}


	ColorOverlay::ColorOverlay(const Color& in_color, const eBlendMode in_blendMode) :
		color(in_color), blendMode(in_blendMode) {}

	void ColorOverlay::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		Color* pEnd = src + srcSize.width * srcSize.height;

		while (src != pEnd) {
			blend(*dst, *src, color, blendMode);
			dst->a = src->a; // Mask overlay
			++src; ++dst;
		}
	}


	Blur::Blur(const int in_size) : size(in_size) {}

	void Blur::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {

		int accum[4];
		int sizeTotal = size * 2 + 1;
		Color* blurImg = new Color[srcSize.width * srcSize.height];

		// Blur U
		Color* pCur = src + size;
		Color* pLookup = src;
		Color* pLookupIter;
		Color* pEnd = src + srcSize.width * srcSize.height - size;
		Color* pDst = blurImg + size;
		while (pCur != pEnd) {
			accum[0] = pLookup->r;
			accum[1] = pLookup->g;
			accum[2] = pLookup->b;
			accum[3] = pLookup->a;

			pLookupIter = pLookup + 1;
			for (int i = 1; i < sizeTotal; ++i, ++pLookupIter) {
				accum[0] += pLookupIter->r;
				accum[1] += pLookupIter->g;
				accum[2] += pLookupIter->b;
				accum[3] += pLookupIter->a;
			}

			pDst->r = accum[0] / sizeTotal;
			pDst->g = accum[1] / sizeTotal;
			pDst->b = accum[2] / sizeTotal;
			pDst->a = accum[3] / sizeTotal;

			++pCur;
			++pLookup;
			++pDst;
		}

		// Blur V
		pCur = blurImg + size * srcSize.width;
		pLookup = blurImg;
		pLookupIter;
		pEnd = blurImg + srcSize.width * srcSize.height - size * srcSize.width;
		pDst = dst + size * srcSize.width;
		while (pCur != pEnd) {
			accum[0] = pLookup->r;
			accum[1] = pLookup->g;
			accum[2] = pLookup->b;
			accum[3] = pLookup->a;

			pLookupIter = pLookup + srcSize.width;
			for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width) {
				accum[0] += pLookupIter->r;
				accum[1] += pLookupIter->g;
				accum[2] += pLookupIter->b;
				accum[3] += pLookupIter->a;
			}

			pDst->r = accum[0] / sizeTotal;
			pDst->g = accum[1] / sizeTotal;
			pDst->b = accum[2] / sizeTotal;
			pDst->a = accum[3] / sizeTotal;

			++pCur;
			++pLookup;
			++pDst;
		}

		delete[] blurImg;
	}



	Outline::Outline(const Color& in_color, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), size(in_size), blendMode(in_blendMode) {}

	void Outline::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur fxBlur(size);
		fxBlur.apply(NULL, blurImg, src, srcSize);

		// Use the blur to create our outline
		Color* pEnd = blurImg + srcSize.width * srcSize.height;
		Color* pBlurPx = blurImg;
		Color final = color;
		int sizeP2 = 1;
		while (sizeP2 < size) sizeP2 *= 2;
		if (sizeP2 > 32) sizeP2 = 32;
		int divider = 32 / sizeP2;
		int multiplier = 8 * sizeP2;
		while (pBlurPx != pEnd) {
			// Some magic to transform the blur into outline
			final.a = dle::clamp(pBlurPx->a, 0, divider);
			final.a = dle::min(255, final.a * multiplier);
			final.a = (final.a * color.a) / 255;
			blend(*baseLayer, *baseLayer, final, blendMode); // Blend direction to base layer.
			++pBlurPx; ++src; ++baseLayer;
		}

		delete[] blurImg;
	}


	Shadow::Shadow(const Color& in_color, const Offset& in_offset, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), offset(in_offset), size(in_size), blendMode(in_blendMode) {}

	void Shadow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur fxBlur(size);
		fxBlur.apply(NULL, blurImg, src, srcSize);

		// Use the blur to create our shadow, using the offset
		Color* pEnd;
		Color* pBlurPx = blurImg;
		Color final = color;
		if (offset.x < 0) {
			pBlurPx -= offset.x;
			pEnd = baseLayer + srcSize.width * srcSize.height + offset.x;
		}
		else {
			baseLayer += offset.x;
			src += offset.x;
			pEnd = baseLayer + srcSize.width * srcSize.height;
		}
		if (offset.y < 0) {
			pBlurPx -= offset.y * srcSize.width;
			pEnd += offset.y * srcSize.width;
		}
		else {
			baseLayer += offset.y * srcSize.width;
			src += offset.y * srcSize.width;
		}
		while (baseLayer != pEnd) {
			final.a = (pBlurPx->a * color.a) / 255;
			blend(*baseLayer, *baseLayer, final, blendMode); // Blend direction to base layer.
			++baseLayer; ++src; ++pBlurPx;
		}

		delete[] blurImg;
	}



	InnerShadow::InnerShadow(const Color& in_color, const Offset& in_offset, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), offset(in_offset), size(in_size), blendMode(in_blendMode) {}

	void InnerShadow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur fxBlur(size);
		fxBlur.apply(NULL, blurImg, src, srcSize);

		// Use the blur to create our shadow, using the offset
		Color* pEnd;
		Color* pBlurPx = blurImg;
		Color final = color;
		if (offset.x < 0) {
			pBlurPx -= offset.x;
			pEnd = dst + srcSize.width * srcSize.height + offset.x;
		}
		else {
			dst += offset.x;
			src += offset.x;
			pEnd = dst + srcSize.width * srcSize.height;
		}
		if (offset.y < 0) {
			pBlurPx -= offset.y * srcSize.width;
			pEnd += offset.y * srcSize.width;
		}
		else {
			dst += offset.y * srcSize.width;
			src += offset.y * srcSize.width;
		}
		while (dst != pEnd) {
			final.a = ((255 - pBlurPx->a) * color.a) / 255;
			final.a = final.a * src->a / 255;
			blend(*dst, *src, final, blendMode);
			++dst; ++src; ++pBlurPx;
		}

		delete[] blurImg;
	}



	Glow::Glow(const Color& in_color, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), size(in_size), blendMode(in_blendMode) {}

	void Glow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur fxBlur(size);
		fxBlur.apply(NULL, blurImg, src, srcSize);

		Color* pBlurPx = blurImg;
		Color* pEnd = baseLayer + srcSize.width * srcSize.height;
		Color final = color;
		while (baseLayer != pEnd) {
			final.a = pBlurPx->a;
			final.a = dle::clamp(final.a, 0, 128);
			final.a = dle::min(255, final.a * 2);
			final.a = final.a * color.a / 255;
			blend(*baseLayer, *baseLayer, final, blendMode);
			++baseLayer; ++src; ++pBlurPx;
		}

		delete[] blurImg;
	}



	InnerGlow::InnerGlow(const Color& in_color, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), size(in_size), blendMode(in_blendMode) {}

	void InnerGlow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur fxBlur(size);
		fxBlur.apply(NULL, blurImg, src, srcSize);

		Color* pBlurPx = blurImg;
		Color* pEnd = dst + srcSize.width * srcSize.height;
		Color final = color;
		while (dst != pEnd) {
			final.a = pBlurPx->a;
			final.a = dle::clamp(final.a, 127, 255) - 127;
			final.a = 255 - dle::min(255, final.a * 2);
			final.a = final.a * color.a / 255;
			final.a = final.a * src->a / 255;
			blend(*dst, *dst, final, blendMode);
			++dst; ++src; ++pBlurPx;
		}

		delete[] blurImg;
	}


	Gradient::Gradient(const std::vector<GradientKey>& in_keys, int in_angle, const eBlendMode in_blendMode) :
		keys(in_keys), angle(wrapAngle(in_angle)), blendMode(in_blendMode) {}

	void Gradient::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const {
		if (!keys.size()) return;

		Color* pCur = src;
		const Color* pEnd = src + srcSize.width * srcSize.height;
		int x = 0;
		int y = 0;
		int percent, localPercent;
		Color final;

		auto* pKey = &keys[0];
		const auto* pKeyEnd = pKey + keys.size();
		const int sintheta = g_sintable[angle] / 100;
		const int costheta = g_sintable[(angle + 90) % 360] / 100;
		const int size = abs(sintheta * srcSize.width) + abs(costheta * srcSize.height);
		while (pCur != pEnd) {
			while (x != srcSize.width) {
				if (sintheta >= 0) {
					if (costheta >= 0) {
						percent = (x * sintheta + y * costheta) * 10000 / size;
					}
					else {
						percent = (x * sintheta + (srcSize.height - y) * -costheta) * 10000 / size;
					}
				}
				else {
					if (costheta >= 0) {
						percent = ((srcSize.width - x) * -sintheta + y * costheta) * 10000 / size;
					}
					else {
						percent = ((srcSize.width - x) * -sintheta + (srcSize.height - y) * -costheta) * 10000 / size;
					}
				}
				percent = dle::min(percent, 10000);
				percent = dle::max(percent, 0);
				localPercent = 0;

				pKey = &keys[0];
				while (pKey != pKeyEnd) {
					if (percent < pKey->percent * 100) {
						percent = (percent - localPercent * 100) * 10000 / (pKey->percent * 100 - localPercent * 100);
						lerpPercentile(final, final, pKey->color, percent);
						break;
					}
					final = pKey->color;
					localPercent = pKey->percent;
					++pKey;
				}

				final.a = pCur->a * final.a / 255;
				blend(*dst, *dst, final, blendMode);

				++pCur; ++dst;
				++x;
			}
			x = 0;
			++y;
		}
	}

	Layer::~Layer() {
		for (auto* pEffect : effects) {
			delete pEffect;
		}
		delete[] src;
	}

	void Layer::bake(void* dst) const {
		bake((Color*) dst);
	}

	void Layer::bake(Color* dst) const {
		int len = size.width * size.height;
		Color* tmpImg = new Color[len * 2];
		Color* tmpSrc = tmpImg + len;

		// Copy our layer into temp buffer. We will apply the effects on top of it
		memcpy(tmpImg, src, sizeof(Color) * len);

		// Bake all effects
		for (auto* pEffect : effects) {
			memcpy(tmpSrc, tmpImg, sizeof(Color) * len);
			pEffect->apply(dst, tmpImg, tmpSrc, size);
		}

		// Bake the final layer image onto the destination, using proper blending
		Color* src = tmpImg;
		Color* pEnd = dst + len;
		while (dst != pEnd) {
			blend(*dst, *dst, *src, blendMode);
			++dst; ++src;
		}

		delete[] tmpImg;
	}


	void applyLayers(void* dst, const Size& srcSize, const Layer& layer) {
		assert(
			layer.size.width == srcSize.width &&
			layer.size.height == srcSize.height &&
			"All layers must match the output dimensions");

		layer.bake((Color*) dst);
	}
	
}
