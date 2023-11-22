#pragma once
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace Constants
		{
			//graphic root
			static constexpr int objCBIndex = 0;
			static constexpr int skinCBIndex = objCBIndex + 1;
			static constexpr int enginePassCBIndex = skinCBIndex + 1;
			static constexpr int scenePassCBIndex = enginePassCBIndex + 1;
			static constexpr int camCBIndex = scenePassCBIndex + 1;
			//static constexpr int litIndexCBIndex = camCBIndex + 1;
			//static constexpr int boundObjCBIndex = litIndexCBIndex + 1;

			static constexpr int dLitBuffIndex = camCBIndex + 1;
			static constexpr int pLitBuffIndex = dLitBuffIndex + 1;
			static constexpr int sLitBuffIndex = pLitBuffIndex + 1;
			static constexpr int rLitBuffIndex = sLitBuffIndex + 1;
			static constexpr int csmBuffIndex = rLitBuffIndex + 1;
			static constexpr int matBuffIndex = csmBuffIndex + 1;

			static constexpr int texture2DBuffIndex = matBuffIndex + 1;
			static constexpr int textureCubeBuffIndex = texture2DBuffIndex + 1;
			static constexpr int textureShadowMapBuffIndex = textureCubeBuffIndex + 1;
			static constexpr int textureShadowMapArrayBuffIndex = textureShadowMapBuffIndex + 1;
			static constexpr int textureShadowMapCubeBuffIndex = textureShadowMapArrayBuffIndex + 1;
			//static constexpr int textureNormalMapIndex = textureShadowMapCubeBuffIndex + 1;

			static constexpr int graphicRootSignatureSlotCount = textureShadowMapCubeBuffIndex + 1;
		}
	}
}