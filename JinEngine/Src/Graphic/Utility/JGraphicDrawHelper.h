#pragma once
#include"../../Core/JDataType.h"

struct ID3D12Resource;

namespace JinEngine
{
	class JScene;
	class JLight;
	class JCamera;
	namespace Graphic
	{
		struct JGraphicDrawHelper
		{
		public:
			JScene* scene = nullptr;
			JCamera* cam = nullptr;
			JLight* lit = nullptr; 
		public:
			uint objectMeshOffset = 0;
			uint objectRitemOffset = 0;
			uint passOffset = 0;
			uint aniOffset = 0;
			uint camOffset = 0;
			uint litIndexOffset = 0;
			uint shadowOffset = 0;
		public:
			bool allowDrawDebug = false;
			bool allowCulling = true;
		public:
			//bool isDrawShadowMap = false;
			//bool isAnimationActivated = false;
		};
	}
}