#pragma once
#include"../../Core/JDataType.h"  
#include"../../../Lib/DirectX/d3dx12.h"

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
			//draw outline and debug layer object
			bool allowDrawDebug = false;
			//execute frustum and occlusion culling
			bool allowCulling = true; 
		public: 
			CD3DX12_CPU_DESCRIPTOR_HANDLE mainDsv;
			CD3DX12_CPU_DESCRIPTOR_HANDLE mainRtv;
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE editorDsv; 
		public:
			CD3DX12_CPU_DESCRIPTOR_HANDLE* dsvArr[2];	// main .. editor
			CD3DX12_CPU_DESCRIPTOR_HANDLE* rtvArr[2];	// main .. editor
		public:
			void StuffHandleArray()noexcept
			{
				dsvArr[0] = &mainDsv;
				dsvArr[1] = &editorDsv;
				rtvArr[0] = &mainRtv;
				rtvArr[1] = nullptr;
			}
		};
	}
}