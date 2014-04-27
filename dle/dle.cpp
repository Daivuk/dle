#include <stdlib.h>
#include "dle.h"
#include "dletables.h"
#include <assert.h>


namespace dle {
	class Pool {
	public:
		Pool();
		virtual ~Pool();

		void* alloc();

		template <class T> void free(T* in_pObj) {
			in_pObj->~T();
			unsigned char* pPtr = (unsigned char*)in_pObj;
			*(pPtr + m_biggestObj) = 0;
			--m_currentAlloc;
		}

		template <class T> void registerType() {
			int typeSize = sizeof(T);
			if (m_biggestObj < typeSize) m_biggestObj = typeSize;
		}

		void create(int in_count);

		int getTotal() const { return m_total; }
		int getCurrentAlloc() const { return m_currentAlloc; }
		int getChunkSize() const { return m_biggestObj + s_headerSize; }
		void* getData() const { return m_pData; }
		void clear();

	private:
		static int					s_headerSize;

		int							m_biggestObj;
		int							m_current;
		int							m_currentAlloc;
		int							m_total;
		unsigned char*				m_pData;
		unsigned char*				m_pAlignedData;
	};

	int Pool::s_headerSize = 1;

	Pool::Pool() :
		m_biggestObj(0),
		m_current(0),
		m_total(0),
		m_pData(0),
		m_pAlignedData(0),
		m_currentAlloc(0) {
	}

	Pool::~Pool() {
		if (m_pData) delete[](unsigned long*)m_pData;
	}

	void* Pool::alloc() {
		int start = m_current++;
		if (m_current == m_total) m_current = 0;
		while (m_current != start) {
			unsigned char* pPtr = m_pData + (m_biggestObj + s_headerSize) * m_current;
			if (*(pPtr + m_biggestObj) == 0) {
				*(pPtr + m_biggestObj) = 1;
				++m_currentAlloc;
				return pPtr;
			}
			++m_current;
			if (m_current == m_total) m_current = 0;
		}
		assert(0 && "No more room. allocate more space.");
		return 0;
	}


	void Pool::create(int in_count)	{
		m_total = in_count;
		int sizePerObj = m_biggestObj + s_headerSize;
		while (sizePerObj % sizeof(long)) ++sizePerObj;
		m_biggestObj = sizePerObj - s_headerSize;
		m_pData = (unsigned char*)(new long[(m_biggestObj + s_headerSize) * m_total / sizeof(long)]);
		memset(m_pData, 0, (m_biggestObj + s_headerSize) * m_total);
	}

	void Pool::clear() {
		m_currentAlloc = 0;
		memset(m_pData, 0, (m_biggestObj + s_headerSize) * m_total);
	}


	// Buffer used for temporary data. Used in certain effect like Blur
	static bool				g_isPoolInited = false;
	static Pool				g_effectPool;

	void lazyInitPool() {
		if (g_isPoolInited) return;
		g_effectPool.registerType<ColorOverlay>();
		g_effectPool.registerType<Blur>();
		g_effectPool.registerType<Outline>();
		g_effectPool.registerType<Shadow>();
	/*	g_effectPool.registerType<InnerShadow>();
		g_effectPool.registerType<Glow>();
		g_effectPool.registerType<InnerGlow>();
		g_effectPool.registerType<Gradient>();*/
		g_effectPool.create(1000);
	}

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

	inline void lerpPercentile(Color& out, const Color& a, const Color& b, const int t)	{
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
		switch (blendmode) {
			case kBlendMode_Normal:
				out.r = dle::min(255, src.r * src.a / 255 + dst.r * (255 - src.a) / 255);
				out.g = dle::min(255, src.g * src.a / 255 + dst.g * (255 - src.a) / 255);
				out.b = dle::min(255, src.b * src.a / 255 + dst.b * (255 - src.a) / 255);
				out.a = dle::max(dst.a, src.a);
				break;
			case kBlendMode_Multiply:
				out.r = dst.r * src.r / 255;
				out.g = dst.g * src.g / 255;
				out.b = dst.b * src.b / 255;
				out.a = src.a;
				lerpPreserveAlpha(out, dst, out, src.a);
				break;
			case kBlendMode_Additive:
				out.r = dle::min(255, dst.r + src.r * src.a / 255);
				out.g = dle::min(255, dst.g + src.g * src.a / 255);
				out.b = dle::min(255, dst.b + src.b * src.a / 255);
				out.a = dle::max(dst.a, src.a);
				break;
		}
	}

	void Object::release() {
		assert((m_refCount > 0) && "Reference count should not be zero");
		if (!--m_refCount) dealloc();
	}

	void Object::retain() {
		++m_refCount;
	}


	void Effect::dealloc() {
		g_effectPool.free(this);
	}


	ColorOverlay* ColorOverlay::create(const Color& color, const eBlendMode blendMode) {
		lazyInitPool();
		ColorOverlay* pColorOverlay = new (g_effectPool.alloc()) ColorOverlay();
		pColorOverlay->color = color;
		pColorOverlay->blendMode = blendMode;
		return pColorOverlay;
	}

	void ColorOverlay::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) {
		Color* pEnd = src + srcSize.width * srcSize.height;

		while (src != pEnd) {
			blend(*dst, *src, color, blendMode);
			dst->a = src->a; // Mask overlay
			++src; ++dst;
		}
	}


	Blur* Blur::create(const int size) {
		lazyInitPool();
		Blur* pBlur = new (g_effectPool.alloc()) Blur();
		pBlur->size = size;
		return pBlur;
	}

	void Blur::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) {

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


	
	Outline* Outline::create(const Color& color, const int size, const eBlendMode blendMode) {
		lazyInitPool();
		Outline* pOutline = new (g_effectPool.alloc()) Outline();
		pOutline->color = color;
		pOutline->size = size;
		pOutline->blendMode = blendMode;
		return pOutline;
	}

	void Outline::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur* pBlur = dle::Blur::create(size);
		pBlur->apply(NULL, blurImg, src, srcSize);
		pBlur->release();

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


	Shadow* Shadow::create(const Color& color, const Offset& offset, const int size, const eBlendMode blendMode) {
		lazyInitPool();
		Shadow* pShadow = new (g_effectPool.alloc()) Shadow();
		pShadow->color = color;
		pShadow->offset = offset;
		pShadow->size = size;
		pShadow->blendMode = blendMode;
		return pShadow;
	}

	void Shadow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur* pBlur = dle::Blur::create(size);
		pBlur->apply(NULL, blurImg, src, srcSize);
		pBlur->release();

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



	InnerShadow* InnerShadow::create(const Color& color, const Offset& offset, const int size, const eBlendMode blendMode) {
		lazyInitPool();
		InnerShadow* pInnerShadow = new (g_effectPool.alloc()) InnerShadow();
		pInnerShadow->color = color;
		pInnerShadow->offset = offset;
		pInnerShadow->size = size;
		pInnerShadow->blendMode = blendMode;
		return pInnerShadow;
	}

	void InnerShadow::apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) {
		// We create a blur first
		Color* blurImg = new Color[srcSize.width * srcSize.height];
		memset(blurImg, 0, sizeof(Color) * srcSize.width * srcSize.height);
		dle::Blur* pBlur = dle::Blur::create(size);
		pBlur->apply(NULL, blurImg, src, srcSize);
		pBlur->release();

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

/*
	Glow::Glow(const Color& in_color, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), size{ in_size }, blendMode{ in_blendMode } {};

	void Glow::apply(Color* src, const Size& srcSize)
	{
		lazyInitTmpBuf();

		int accum;
		int sizeTotal = size * 2 + 1;
		Color* pLookupIter;
		Color* pCur;
		Color* pLookup;
		Color* pEnd;
		Color* pDst;

		// Blur U, only care about alpha component
		pCur = src + size;
		pLookup = src;
		pEnd = src + srcSize.width * srcSize.height - size;
		pDst = g_tmpBuf + size;
		while (pCur < pEnd)
		{
			accum = pLookup->a;

			pLookupIter = pLookup + 1;
			for (int i = 1; i < sizeTotal; ++i, ++pLookupIter)
			{
				accum += pLookupIter->a;
			}

			pDst->a = accum / sizeTotal;

			++pCur;
			++pLookup;
			++pDst;
		}

		// Blur V, but use shadow color
		pCur = g_tmpBuf + size * srcSize.width;
		pLookup = g_tmpBuf;
		pEnd = g_tmpBuf + srcSize.width * srcSize.height - size * srcSize.width;
		pDst = src + size * srcSize.width;
		Color final = color;
		Color tmp;
		switch (blendMode)
		{
		case kBlendMode_Normal:
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;
				final.a = accum / sizeTotal;
				final.a = (final.a * color.a) / 255;
				lerpPreserveAlpha(*pDst, final, *pDst, pDst->a);
				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		case kBlendMode_Multiply:
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;
				final.a = accum / sizeTotal;
				final.a = (final.a * color.a) / 255;

				lerpPreserveAlpha(tmp, final, *pDst, pDst->a);

				pDst->r = (pCur->r * tmp.r) / 255;
				pDst->g = (pCur->g * tmp.g) / 255;
				pDst->b = (pCur->b * tmp.b) / 255;
				pDst->a = (pCur->a * tmp.a) / 255;

				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		case kBlendMode_Additive:
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;
				final.a = accum / sizeTotal;			
				final.a = dle::clamp(final.a, 0, 128);
				final.a = dle::min(255, final.a * 2);
				final.a = (final.a * color.a) / 255;

				lerpPreserveAlpha(tmp, final, *pDst, pDst->a);

				pDst->r = dle::min(255, (pDst->r * pDst->a) / 255 + tmp.r);
				pDst->g = dle::min(255, (pDst->g * pDst->a) / 255 + tmp.g);
				pDst->b = dle::min(255, (pDst->b * pDst->a) / 255 + tmp.b);
				pDst->a = tmp.a;

				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		}
	}



	InnerGlow::InnerGlow(const Color& in_color, const int in_size, const eBlendMode in_blendMode) :
		color(in_color), size{ in_size }, blendMode{ in_blendMode } {};

	void InnerGlow::apply(Color* src, const Size& srcSize)
	{
		lazyInitTmpBuf();

		int accum;
		int sizeTotal = size * 2 + 1;
		Color* pLookupIter;
		Color* pCur;
		Color* pLookup;
		Color* pEnd;
		Color* pDst;

		// Blur U, only care about alpha component
		pCur = src + size;
		pLookup = src;
		pEnd = src + srcSize.width * srcSize.height - size;
		pDst = g_tmpBuf + size;
		while (pCur < pEnd)
		{
			accum = pLookup->a;

			pLookupIter = pLookup + 1;
			for (int i = 1; i < sizeTotal; ++i, ++pLookupIter)
			{
				accum += pLookupIter->a;
			}

			pDst->a = accum / sizeTotal;

			++pCur;
			++pLookup;
			++pDst;
		}

		// Blur V, but use shadow color
		pCur = g_tmpBuf + size * srcSize.width;
		pLookup = g_tmpBuf;
		pEnd = g_tmpBuf + srcSize.width * srcSize.height - size * srcSize.width;
		pDst = src + size * srcSize.width;
		Color final = color;
		Color tmp;
		switch (blendMode)
		{
		case kBlendMode_Normal:
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;
				final.a = accum / sizeTotal;
				final.a = (final.a * color.a) / 255;
				lerpPreserveAlpha(tmp, *pDst, final, (255 - final.a) * color.a / 255);
				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		case kBlendMode_Multiply:
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;
				final.a = accum / sizeTotal;
				final.a = (final.a * color.a) / 255;

				lerpPreserveAlpha(tmp, *pDst, final, (255 - final.a) * color.a / 255);

				pDst->r = (pCur->r * tmp.r) / 255;
				pDst->g = (pCur->g * tmp.g) / 255;
				pDst->b = (pCur->b * tmp.b) / 255;
				pDst->a = (pCur->a * tmp.a) / 255;

				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		case kBlendMode_Additive:
			color.r = color.r * color.a / 255;
			color.g = color.g * color.a / 255;
			color.b = color.b * color.a / 255;
			while (pCur < pEnd)
			{
				accum = pLookup->a;
				pLookupIter = pLookup + srcSize.width;
				for (int i = 1; i < sizeTotal; ++i, pLookupIter += srcSize.width)
					accum += pLookupIter->a;

				final.r = dle::min(255, color.r + pDst->r);
				final.g = dle::min(255, color.g + pDst->g);
				final.b = dle::min(255, color.b + pDst->b);
				final.a = pDst->a;
				accum = accum / sizeTotal;
				accum = dle::clamp(accum, 128, 255) - 128;
				accum = dle::min(255, accum * 2);

				lerpPreserveAlpha(*pDst, final, *pDst, accum);

				++pCur;
				++pLookup;
				++pDst;
			}
			break;
		}
	}



	Gradient::Gradient(const std::vector<GradientKey>& in_keys, int in_angle) :
		keys{ in_keys }, angle{ wrapAngle(in_angle) } {}

	void Gradient::apply(Color* src, const Size& srcSize)
	{
		if (!keys.size()) return;

		Color* pCur = src;
		Color* pEnd = src + srcSize.width * srcSize.height;
		int percent, localPercent;
		int x = 0;
		int y = 0;
		Color kernelColor;

		if (angle == 0 || angle == 180)
		{
			// This is the cheapest. Only need to check each row
			while (pCur != pEnd)
			{
				if (angle)	percent = ((srcSize.height - y) * 10000) / srcSize.height;
				else		percent = (y * 10000) / srcSize.height;
				localPercent = 0;

				for (auto& key : keys)
				{
					if (percent < key.percent * 100)
					{
						percent = (percent - localPercent * 100) * 10000 / (key.percent * 100 - localPercent * 100);
						lerpPercentile(kernelColor, kernelColor, key.color, percent);
						break;
					}
					kernelColor = key.color;
					localPercent = key.percent;
				}

				while (x != srcSize.width)
				{
					pCur->r = kernelColor.r;
					pCur->g = kernelColor.g;
					pCur->b = kernelColor.b;
					pCur->a = (pCur->a * kernelColor.a) / 255;
					++pCur;
					++x;
				}
				x = 0;
				++y;
			}
		}
		else if (angle == 90 || angle == 270)
		{
			// More costy, need to get the kernel each column
			auto* pKey = &keys[0];
			auto* pKeyEnd = pKey + keys.size();
			while (pCur != pEnd)
			{
				while (x != srcSize.width)
				{
					if (angle == 270)	percent = ((srcSize.width - x) * 10000) / srcSize.width;
					else				percent = (x * 10000) / srcSize.width;
					localPercent = 0;

					pKey = &keys[0];
					while (pKey != pKeyEnd)
					{
						if (percent < pKey->percent * 100)
						{
							percent = (percent - localPercent * 100) * 10000 / (pKey->percent * 100 - localPercent * 100);
							lerpPercentile(kernelColor, kernelColor, pKey->color, percent);
							break;
						}
						kernelColor = pKey->color;
						localPercent = pKey->percent;
						++pKey;
					}

					pCur->r = kernelColor.r;
					pCur->g = kernelColor.g;
					pCur->b = kernelColor.b;
					pCur->a = (pCur->a * kernelColor.a) / 255;
					++pCur;
					++x;
				}
				x = 0;
				++y;
			}
		}
		else
		{
			// Any other angles are more costly
			auto* pKey = &keys[0];
			auto* pKeyEnd = pKey + keys.size();
			int sintheta = g_sintable[angle] / 100;
			int costheta = g_costable[angle] / 100;
			int size = abs(sintheta * srcSize.width) + abs(costheta * srcSize.height);
			while (pCur != pEnd)
			{
				while (x != srcSize.width)
				{
					if (sintheta >= 0) 	
					{
						if (costheta >= 0) 
						{
							percent = (x * sintheta + y * costheta) * 10000 / size;
						}
						else
						{
							percent = (x * sintheta + (srcSize.height - y) * -costheta) * 10000 / size;
						}
					}
					else
					{
						if (costheta >= 0)
						{
							percent = ((srcSize.width - x) * -sintheta + y * costheta) * 10000 / size;
						}
						else
						{
							percent = ((srcSize.width - x) * -sintheta + (srcSize.height - y) * -costheta) * 10000 / size;
						}
					}
					percent = dle::min(percent, 10000);
					percent = dle::max(percent, 0);
					localPercent = 0;

					pKey = &keys[0];
					while (pKey != pKeyEnd)
					{
						if (percent < pKey->percent * 100)
						{
							percent = (percent - localPercent * 100) * 10000 / (pKey->percent * 100 - localPercent * 100);
							lerpPercentile(kernelColor, kernelColor, pKey->color, percent);
							break;
						}
						kernelColor = pKey->color;
						localPercent = pKey->percent;
						++pKey;
					}

					pCur->r = kernelColor.r;
					pCur->g = kernelColor.g;
					pCur->b = kernelColor.b;
					pCur->a = (pCur->a * kernelColor.a) / 255;
					++pCur;
					++x;
				}
				x = 0;
				++y;
			}
		}
	}
	*/


	Layer* Layer::create(unsigned char* src, const Size& size, const eBlendMode blendMode) {
		return create((Color*)src, size, blendMode);
	}
	Layer* Layer::create(Color* src, const Size& size, const eBlendMode blendMode) {
		return create(src, size, {}, blendMode);
	}
	Layer* Layer::create(unsigned char* src, const Size& size, const std::vector<Effect*>& effects, const eBlendMode blendMode) {
		return create((Color*)src, size, effects, blendMode);
	}
	Layer* Layer::create(Color* src, const Size& size, const std::vector<Effect*>& effects, const eBlendMode blendMode) {
		lazyInitPool();
		Layer* pLayer = new (g_effectPool.alloc()) Layer();
		pLayer->src = new Color[size.width * size.height]; //TODO: [dsl] Pool that somehow
		memcpy(pLayer->src, src, sizeof(Color)* size.width * size.height); //TODO: [dsl] Worth threading the memcpy?
		pLayer->size = size;
		pLayer->effects = effects;
		for (auto& effect : pLayer->effects) {
			effect->retain();
		}
		pLayer->blendMode = blendMode;
		return pLayer;
	}

	void Layer::dealloc() {
		g_effectPool.free(this);
	}

	Layer::~Layer() {
		for (auto* pEffect : effects) {
			pEffect->release();
		}
		delete[] src;
	}

	void Layer::bake(Color* dst) const {
		int len = size.width * size.height;
		Color* tmpImg = new Color[len * 2];
		Color* tmpSrc = tmpImg + len;

		// Copy our layer into temp buffer. We will apply the effects on top of it
		memcpy(tmpImg, src, sizeof(Color)* len);

		// Bake all effects
		for (auto* pEffect : effects) {
			memcpy(tmpSrc, tmpImg, sizeof(Color)* len);
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



	void bake(unsigned char* dst, const std::vector<Layer*>& layers) {
		bake((Color*)dst, layers);
	}

	void bake(Color* dst, const std::vector<Layer*>& layers) {
		if (!layers.size()) return;

		const Size& fixedSize = layers.front()->size;

		// Apply all layers in order
		for (const auto* pLayer : layers) {
			assert(
				pLayer->size.width == fixedSize.width &&
				pLayer->size.height == fixedSize.height &&
				"All layers must be of exact same dimensions");

			pLayer->bake(dst);
		}
	}

}
