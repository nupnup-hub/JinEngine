#pragma once
#include<DirectXCollision.h> 
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Utility/JVector.h"

namespace JinEngine
{ 
	class JCamera;
	namespace Graphic
	{
		struct JGraphicInfo;
		class JHardwareOccCulling
		{
		private:
			JUserPtr<JCamera> camera;
			DirectX::BoundingFrustum updateFrustum;
		private:
			JVector3<float> prePos;
			JVector3<float> preRot;
		private:
			float posFactor = 2;
			float rotFactor = 15;
		public:
			void Initialize();
			void Clear();
		};
	}
}
