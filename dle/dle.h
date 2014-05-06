#ifndef DLE_H_INCLUDED
#define DLE_H_INCLUDED

#include <vector>

namespace dle
{
	/**
		Blend modes enum. All blend values are calculated, then interpolated
		by the source opacity.

		s = Source color, top layer
		d = Destination color, underlying layer

		Only commented ones are implemented so far.

		TODO: This will have to change into types, so we can use templates to
		boost performances in tight loops.
	*/
	enum eBlendMode	{
		kBlendMode_Normal,			/**< f(sd) = s */
		kBlendMode_Dissolve,

		kBlendMode_Darken,
		kBlendMode_Multiply,		/**< f(sd) = s * d */
		kBlendMode_ColorBurn,
		kBlendMode_LinearBurn,
		kBlendMode_DarkerColor,

		kBlendMode_Lighten,
		kBlendMode_Screen,			/**< f(sd) = 1 - (1 - s) * (1 - d) */
		kBlendMode_ColorDodge,
		kBlendMode_LinearDodge,
		kBlendMode_Additive = kBlendMode_LinearDodge,
		kBlendMode_LighterColor,

		kBlendMode_Overlay,
		kBlendMode_SoftLight,
		kBlendMode_HardLight,
		kBlendMode_VividLight,
		kBlendMode_LinearLight,
		kBlendMode_PinLight,
		kBlendMode_HardMix,

		kBlendMode_Difference,
		kBlendMode_Exclusion,
		kBlendMode_Substract,
		kBlendMode_Divide,

		kBlendMode_Hue,
		kBlendMode_Saturation,
		kBlendMode_Color,
		kBlendMode_Luminosity,
	};

	/**
		Color structure.
	*/
	struct Color {
		unsigned char r, g, b, a;
	};

	/**
		Offset structure
	*/
	struct Offset {
		int x;
		int y;
	};

	/**
		Size structure
	*/
	struct Size	{
		int width;
		int height;
	};

	/**
		Rectangle structure
	*/
	struct Rect	{
		int x;
		int y;
		int width;
		int height;
	};

	/**
		Gradient key structure.
		Gradients are formed of multiple keys. With color and percentage along
		the line.
	*/
	struct GradientKey {
		Color color;	/**< Color of the gradient at this key */
		int percent;	/**< Percentage position of this key */
	};

	/**
		Base effect class. Pure virtual, can not be instanciated.
		To create a new effect, derive from it and implement apply()
	*/
	class Effect {
	public:
		/**
			Apply the effect.

			@param baseLayer Pointer to the underlying image, under the current
			layer. Certain effects, like shadow or glow, needs to apply their
			own blending to this layer. Not all effects will care about this
			argument.

			@param dst Destination image. This is the current layer with 
			combined effects that were set before this one. This is were normal
			effects will write to. For special things like glow, look at 
			\a baseLayer .

			@param src Source image. This is the current layer with combined
			effects that were set before this one. It's in for reference and
			seperated buffer from \a dst.

			@param srcSize Size of the image. All buffers passed must be of size
			srcSize.width * srcSize.height
		*/
		virtual void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const = 0;
	};

	/**
		Combine a fill color on top of the layer
	*/
	class ColorOverlay final : public Effect {
	public:
		Color		color;		/**< Color of the overlay */
		eBlendMode	blendMode;	/**< Blend mode to apply \a color to the layer */
		ColorOverlay(const Color& in_color = { 255, 0, 0, 255 }, const eBlendMode in_blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Blurs the layer.

		TODO: Use Gaussian instead of Box Blur.
	*/
	class Blur final : public Effect {
	public:
		int			size;		/**< Size of the blur. 0 = no blur. 5 = 9x9 blur, where {5,5} is the center. */
		Blur(const int size);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Creates an outline around the shape. It uses the alpha
		information to do this.

		TODO: Add InnerOutline and CenterOutline effects.
	*/
	class Outline final : public Effect {
	public:
		Color		color;		/**< Color of the outline */
		int			size;		/**< Thickness of the outline */
		eBlendMode	blendMode;	/**< Blend mode to apply \a color to the underlying image */
		Outline(const Color& color = { 0, 0, 0, 245 }, const int size = 2, const eBlendMode blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Add a drop shadow under the layer
	*/
	class Shadow final : public Effect {
	public:
		Color		color;		/**< Color of the shadow */
		Offset		offset;		/**< Offset. {0,0} means it will be directly under the image. {0,5} will be shifted down by 5 pixels */
		int			size;		/**< Size of the blur. 0 = no blur. 5 = 9x9 blur, where {5,5} is the center. */
		eBlendMode	blendMode;	/**< Blend mode to apply the shadow to the underlying image */
		Shadow(const Color& color = { 0, 0, 0, 255 }, const Offset& offset = { 3, 5 }, const int size = 5, const eBlendMode blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};
	
	/**
		Shadow that appears inside the layer
	*/
	class InnerShadow final : public Effect {
	public:
		Color		color;		/**< Color of the shadow */
		Offset		offset;		/**< Offset. {0,0} means it will be directly over the image. {0,5} will be shifted down by 5 pixels and visible at the top */
		int			size;		/**< Size of the blur. 0 = no blur. 5 = 9x9 blur, where {5,5} is the center. */
		eBlendMode	blendMode;	/**< Blend mode to apply the shadow to the layer */
		InnerShadow(const Color& color = { 0, 0, 0, 245 }, const Offset& offset = { 3, 3 }, const int size = 3, const eBlendMode in_blendMode = kBlendMode_Multiply);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Adds a glow around the image using the alpha information
	*/
	class Glow final : public Effect {
	public:
		Color		color;		/**< Color of the glow */
		int			size;		/**< Size of the glow from the edges */
		eBlendMode	blendMode;	/**< Blend mode to apply the glow to the underlying image */
		Glow(const Color& color = { 255, 255, 190, 150 }, const int size = 5, const eBlendMode blendMode = kBlendMode_Screen);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Adds a glow inside the layer, from the edges

		TODO: Add CenterGlow effect to create a glow from the center of the layer
	*/
	class InnerGlow final : public Effect {
	public:
		Color		color;		/**< Color of the glow */
		int			size;		/**< Size of the glow from the edges */
		eBlendMode	blendMode;	/**< Blend mode to apply the glow to the layer */
		InnerGlow(const Color& color = {255, 255, 190, 150}, const int size = 5, const eBlendMode blendMode = kBlendMode_Screen);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		This is like ColorOverlay, but uses a linear gradient instead
		of a fill color.

		TODO: Add RadialGradient effect
	*/
	class Gradient final : public Effect {
	public:
		std::vector<GradientKey> keys;	/**< Gradient key frames defining the color layers of the gradient */
		int angle;						/**< Angle of the gradient. 0 is North, then goes Counter-Clockwise*/
		eBlendMode blendMode;			/**< Blend mode to apply the gradient to the layer */
		Gradient(const std::vector<GradientKey>& keys = {}, int angle = 0, const eBlendMode blendMode = kBlendMode_Normal);
		void apply(Color* baseLayer, Color* dst, Color* src, const Size& srcSize) const;
	};

	/**
		Defines a layer, that can contain multple Effects.
		Effect can be duplicated. This can be use to create interresting
		effects like a rainbow Outline using multiple outlines.

		Layers can be use to compose a final image using multiple blending types with different images
		on top of each others.

		TODO: Add sub-layers
	*/
	class Layer {
	public:
		Size				size;		/**< Dimension of the layer. All layers in a same process should be of the exact same size */
		eBlendMode			blendMode;	/**< Blend mode to apply the layer to the underlying layer */

		/**
			Constructor

			@param in_src Source image. This should be 32 bits per pixel, with components RGBA in this exact order.
			If uncertain, use the Color structure to build your image.

			@param in_size Size of the source image. \a in_src should be of size in_size.width * in_size.height

			@param in_blendMode Blend mode to apply the layer to the underlying layer
		*/
		Layer(const void* in_src, const Size& in_size, const eBlendMode in_blendMode = kBlendMode_Normal) :
			size(in_size), blendMode(in_blendMode) {
			src = new Color[size.width * size.height];
			memcpy(src, in_src, sizeof(Color) * size.width * size.height);
		}

		/**
			Constructor

			@param in_src Source image. This should be 32 bits per pixel, with components RGBA in this exact order.
			If uncertain, use the Color structure to build your image.

			@param in_size Size of the source image. \a in_src should be of size in_size.width * in_size.height

			@param in_blendMode Blend mode to apply the layer to the underlying layer

			@param effects List of effects. i.e: dle::Shadow(), dle::Outline(), dle::ColorOverlay(), ...
			Those effects will be applied in the same order that they are added to the layer.
		*/
		template<typename... Effects> Layer(const void* in_src, const Size& in_size, const eBlendMode in_blendMode, const Effects&... effects) : size(in_size), blendMode(in_blendMode) {
			src = new Color[size.width * size.height];
			memcpy(src, in_src, sizeof(Color) * size.width * size.height);
			addEffect(effects...);
		}

		/**
			Virtual destructor.
		*/
		virtual ~Layer();

		/**
			Add effects to the layer

			@param effect and effects; List of effects. i.e: dle::Shadow(), dle::Outline(), dle::ColorOverlay(), ...
			Those effects will be applied in the same order that they are added to the layer.
		*/
		template<typename T, typename... Effects> void addEffect(const T& effect, const Effects&... effects) {
			addEffect(effect);
			addEffect(effects...);
		}

		/**
			Add an effect to the layer

			@param effect Effect to add
		*/
		template<typename T> void addEffect(const T& effect) {
			T* pEffect = new T();
			*pEffect = effect;
			effects.push_back(pEffect);
		}

		/**
			Bake all the effects of the layer to the destination buffer

			@param dst Destination buffer for the layer to be baked to
		*/
		virtual void bake(Color* dst) const;
		void bake(void* dst) const;

	protected:
		Color*					src;
		std::vector<Effect*>	effects;
	};

	/**
		Apply effects to an image buffer directly, without using layers.

		@param dstAndSrc Both source and destination image. The original image will
		be overriden.

		@param srcSize Size of the source image
		
		@param effects List of effects. i.e: dle::Shadow(), dle::Outline(), dle::ColorOverlay(), ...
			Those effects will be applied in the same order that they are passed in
	*/
	template<typename... Effects> void applyEffects(void* dstAndSrc, const Size& srcSize, const Effects&... effects) {
		Layer layer((Color*) dstAndSrc, srcSize, kBlendMode_Normal, effects...);
		layer.bake((Color*) dstAndSrc);
	}

	/**
		Apply effects to an image buffer directly, without using layers.

		@param dst Destination image where the final result will be stored

		@param src Source image. This buffer will be left untouched

		@param srcSize Size of the source image

		@param effects List of effects. i.e: dle::Shadow(), dle::Outline(), dle::ColorOverlay(), ...
		Those effects will be applied in the same order that they are passed in
	*/
	template<typename... Effects> void applyEffects(void* dst, void* src, const Size& srcSize, const Effects&... effects) {
		Layer layer((Color*) src, srcSize, kBlendMode_Normal, effects...);
		layer.bake((Color*) dst);
	}

	/**
		Apply multiple layers to an image buffer

		@param dst Destination image where the final result will be stored

		@param srcSize Size of the destination image and all the layers included.
		All layers must respect that size or an assert will be thrown.

		@param layer and layers; List of layers. They will be applied in the order that they are passed in.
	*/
	template<typename... Layers> void applyLayers(void* dst, const Size& srcSize, const Layer& layer, const Layers&... layers) {
		applyLayers(dst, srcSize, layer);
		applyLayers(dst, srcSize, layers...);
	}
	void applyLayers(void* dst, const Size& srcSize, const Layer& layer);
}

#endif
