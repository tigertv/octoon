#ifndef OCTOON_RENDER_CONTEXT_H_
#define OCTOON_RENDER_CONTEXT_H_

#include <octoon/hal/graphics_data.h>
#include <octoon/hal/graphics_texture.h>

namespace octoon::video
{
	struct HemisphereLight {
		math::float3 direction;
		math::float3 skyColor;
		math::float3 groundColor;
	};

	struct RectAreaLight {
		math::float4 color;
		math::float4 position;
		math::float4 halfWidth;
		math::float4 halfHeight;
	};

	struct SpotLight {
		math::float4 position;
		math::float4 direction;
		math::float4 color;
		float distance;
		float decay;
		float coneCos;
		float penumbraCos;

		int shadow;
		float shadowBias;
		float shadowRadius;
		math::float2 shadowMapSize;
	};

	struct EnvironmentLight {
		float intensity;
		hal::GraphicsTextureWeakPtr radiance;
	};

	struct PointLight {
		math::float4 position;
		math::float4 color;
		float distance;
		float decay;

		int shadow;
		float shadowBias;
		float shadowRadius;
		math::float2 shadowMapSize;
	};

	struct DirectionalLight {
		math::float4 direction;
		math::float3 color;
		int shadow;
		float shadowBias;
		float shadowRadius;
		math::float2 shadowMapSize;
	};

	struct LightModule
	{
		std::size_t numDirectional;
		std::size_t numSpot;
		std::size_t numRectangle;
		std::size_t numPoint;
		std::size_t numHemi;
		std::size_t numEnvironment;

		math::float3 ambientLightColors;

		std::vector<PointLight> pointLights;
		std::vector<SpotLight> spotLights;
		std::vector<RectAreaLight> rectangleLights;
		std::vector<DirectionalLight> directionalLights;
		std::vector<EnvironmentLight> environmentLights;

		std::vector<hal::GraphicsTexturePtr> pointShadows;
		std::vector<hal::GraphicsTexturePtr> spotShadows;
		std::vector<hal::GraphicsTexturePtr> rectangleShadows;
		std::vector<hal::GraphicsTexturePtr> directionalShadows;
		std::vector<hal::GraphicsTexturePtr> environmentShadows;

		std::vector<math::float4x4> directionalShadowMatrix;

		hal::GraphicsDataPtr spotLightBuffer;
		hal::GraphicsDataPtr pointLightBuffer;
		hal::GraphicsDataPtr rectangleLightBuffer;
		hal::GraphicsDataPtr directionLightBuffer;
	};

	class RenderProfile
	{
	public:
		LightModule light;

		void reset() noexcept;
	};
}

#endif