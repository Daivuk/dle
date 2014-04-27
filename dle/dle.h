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

	class Pool;

	class Object {
	public:
		void release();
		void retain();
	protected:
		virtual void dealloc() = 0;
		int m_refCount = 1;
	};

	class Effect : public Object {
	public:
		virtual void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) = 0;
	protected:
		void dealloc();
	};

	class ColorOverlay : public Effect {
	public:
		Color		color;
		eBlendMode	blendMode;
		static ColorOverlay* create(const Color& in_color = { 255, 0, 0, 255 }, const eBlendMode in_blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		ColorOverlay() {}
		~ColorOverlay() {}
	};

	class Blur : public Effect {
	public:
		int			size;
		static Blur* create(const int size);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		Blur() {}
		~Blur() {}
	};

	class Outline : public Effect {
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		static Outline* create(const Color& color = { 0, 0, 0, 245 }, const int size = 2, const eBlendMode blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		Outline() {}
		~Outline() {}
	};

	class Shadow : public Effect {
	public:
		Color		color;
		Offset		offset;
		int			size;
		eBlendMode	blendMode;
		static Shadow* create(const Color& color = {0, 0, 0, 255}, const Offset& offset = { 3, 5 }, const int size = 10, const eBlendMode blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		Shadow() {}
		~Shadow() {}
	};
	
	class InnerShadow : public Effect {
	public:
		Color		color;
		Offset		offset;
		int			size;
		eBlendMode	blendMode;
		static InnerShadow* create(const Color& color = { 0, 0, 0, 245 }, const Offset& offset = { 3, 3 }, const int size = 3, const eBlendMode in_blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		InnerShadow() {}
		~InnerShadow() {}
	};

	class Glow : public Effect {
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		static Glow* create(const Color& color = { 255, 255, 190, 192}, const int size = 5, const eBlendMode blendMode = kBlendMode_Additive);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize);
	private:
		friend Pool;
		Glow() {}
		~Glow() {}
	};
/*
	class InnerGlow : public Effect	{
	public:
		Color		color;
		int			size;
		eBlendMode	blendMode;
		static InnerGlow* create(const Color& in_color, const int in_size, const eBlendMode in_blendMode = kBlendMode_Additive);
		void apply(Color* src, const Size& srcSize);
	private:
		friend Pool;
		InnerGlow() {}
		~InnerGlow() {}
	};

	class Gradient : public Effect {
	public:
		std::vector<GradientKey> keys;
		int angle;
		static Gradient* create(const std::vector<GradientKey>& in_keys, int in_angle);
		void apply(Color* src, const Size& srcSize);
	private:
		friend Pool;
		Gradient() {}
		~Gradient() {}
	};*/


	//-------------------------------------------------------------------------
	// Layers

	class Layer : public Object	{
	public:
		Color*				src;
		Size				size;
		eBlendMode			blendMode;
		static Layer* create(unsigned char* src, const Size& size, const eBlendMode blendMode = kBlendMode_Normal);
		static Layer* create(Color* src, const Size& size, const eBlendMode blendMode = kBlendMode_Normal);
		static Layer* create(unsigned char* src, const Size& size, const std::vector<Effect*>& effects, const eBlendMode blendMode = kBlendMode_Normal);
		static Layer* create(Color* src, const Size& size, const std::vector<Effect*>& effects, const eBlendMode blendMode = kBlendMode_Normal);
		void addEffect(Effect* pEffect);
		void removeEffect(Effect* pEffect);
		void bake(Color* dst) const;
	protected:
		void dealloc();
	private:
		friend Pool;
		std::vector<Effect*>effects;
		Layer() {}
		~Layer();
	};


	//-------------------------------------------------------------------------
	// Functions
	void setMaxImageSize(const int in_maxImageSize = 4096);
	void bake(unsigned char* dst, const std::vector<Layer*>& layers);
	void bake(Color* dst, const std::vector<Layer*>& layers);
}

#endif
