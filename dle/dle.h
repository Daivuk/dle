#ifndef DLE_H_INCLUDED
#define DLE_H_INCLUDED

#include <vector>

namespace dle
{
	//-------------------------------------------------------------------------
	// Types

	enum eBlendMode	{
		kBlendMode_Normal,
		kBlendMode_Additive,
		kBlendMode_Multiply
	};

	struct Color {
		unsigned char r, g, b, a;
	};

	struct Offset {
		int x;
		int y;
	};

	struct Size	{
		int width;
		int height;
	};

	struct Rect	{
		int x;
		int y;
		int width;
		int height;
	};

	struct GradientKey {
		Color color;
		int percent;
	};


	//-------------------------------------------------------------------------
	// Effects

	class Effect {
	public:
		virtual void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const = 0;
	};

	class ColorOverlay : public Effect {
	public:
		Color		color;
		eBlendMode	blendMode;
		ColorOverlay(const Color& in_color = { 255, 0, 0, 255 }, const eBlendMode in_blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class Blur : public Effect {
	public:
		int			size;
		Blur(const int size);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class Outline : public Effect {
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		Outline(const Color& color = { 0, 0, 0, 245 }, const int size = 2, const eBlendMode blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class Shadow : public Effect {
	public:
		Color		color;
		Offset		offset;
		int			size;
		eBlendMode	blendMode;
		Shadow(const Color& color = { 0, 0, 0, 255 }, const Offset& offset = { 3, 5 }, const int size = 10, const eBlendMode blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};
	
	class InnerShadow : public Effect {
	public:
		Color		color;
		Offset		offset;
		int			size;
		eBlendMode	blendMode;
		InnerShadow(const Color& color = { 0, 0, 0, 245 }, const Offset& offset = { 3, 3 }, const int size = 3, const eBlendMode in_blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class Glow : public Effect {
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		Glow(const Color& color = { 255, 255, 190, 150 }, const int size = 5, const eBlendMode blendMode = kBlendMode_Additive);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class InnerGlow : public Effect {
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		InnerGlow(const Color& color = {255, 255, 190, 150}, const int size = 5, const eBlendMode blendMode = kBlendMode_Additive);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	class Gradient : public Effect {
	public:
		std::vector<GradientKey> keys;
		int angle;
		eBlendMode blendMode;
		Gradient(const std::vector<GradientKey>& keys = {}, int angle = 0, const eBlendMode blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};


	//-------------------------------------------------------------------------
	// Layers

	class Layer	{
	public:
		Color*				src;
		Size				size;
		eBlendMode			blendMode;

		Layer(void* in_src, const Size& in_size, const eBlendMode in_blendMode = kBlendMode_Normal) :
			size(in_size), blendMode(in_blendMode) {
			src = new Color[size.width * size.height];
			memcpy(src, in_src, sizeof(Color) * size.width * size.height);
		}
		template<typename... Effects> Layer(void* in_src, const Size& in_size, const eBlendMode in_blendMode, const Effects&... effects) : size(in_size), blendMode(in_blendMode) {
			src = new Color[size.width * size.height];
			memcpy(src, in_src, sizeof(Color) * size.width * size.height);
			addEffect(effects...);
		}
		~Layer();
		template<typename T, typename... Effects> void addEffect(const T& effect, const Effects&... effects) {
			addEffect(effect);
			addEffect(effects...);
		}
		template<typename T> void addEffect(const T& effect) {
			T* pEffect = new T();
			*pEffect = effect;
			effects.push_back(pEffect);
		}
		void bake(void* dst) const;
		void bake(Color* dst) const;
	private:
		std::vector<Effect*> effects;
	};


	//-------------------------------------------------------------------------
	// Functions
	template<typename... Effects> void applyEffects(void* dstAndSrc, const Size& srcSize, const Effects&... effects) {
		Layer layer((Color*) dstAndSrc, srcSize, kBlendMode_Normal, effects...);
		layer.bake((Color*) dstAndSrc);
	}

	template<typename... Effects> void applyEffects(void* dst, void* src, const Size& srcSize, const Effects&... effects) {
		Layer layer((Color*) src, srcSize, kBlendMode_Normal, effects...);
		layer.bake((Color*) dst);
	}

	void applyLayers(void* dst, const Size& srcSize, const Layer& layer);

	template<typename... Layers> void applyLayers(void* dst, const Size& srcSize, const Layer& layer, const Layers&... layers) {
		applyLayers(dst, srcSize, layer);
		applyLayers(dst, srcSize, layers...);
	}
}

#endif
