#pragma once
#include"JCsmOption.h"
#include"JCsmType.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/Geometry/JBBox.h"
#include"../../Core/Math/JMatrix.h"
#include<DirectXCollision.h>
#include<vector>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JCsmTargetInfo; 
		class JCsmManager;
		class JCsmHandlerInterface : public Core::JTypeImplInterfacePointerHolder<JCsmHandlerInterface>
		{
		private:
			friend class JCsmManager;
		private:
			struct ComputeResult
			{
			public:
				JMatrix4x4 shadowProjM[JCsmOption::maxCountOfSplit];
				JVector4<float> scale[JCsmOption::maxCountOfSplit];
				JVector4<float> posOffset[JCsmOption::maxCountOfSplit];
				JVector2<float> frustumSize[JCsmOption::maxCountOfSplit];
				float splitRate[JCsmOption::maxCountOfSplit];
				float fNear[JCsmOption::maxCountOfSplit];
				float fFar[JCsmOption::maxCountOfSplit]; 
			public:
				uint subFrustumCount;
			};
			struct TargetData
			{
			public:
				JOwnerPtr<JCsmTargetInfo> info; 
				ComputeResult result;
			public:
				bool IsValid()const noexcept;
			}; 
		private:  
			mutable JCsmOption option;
			std::vector<TargetData> target; 
		public:
			JCsmOption GetCsmOption()const noexcept;
			uint GetCsmTargetCount()const noexcept; 
			const ComputeResult& GetCsmComputeResult(const uint index)const noexcept;
			virtual size_t GetCsmHandlerGuid()const noexcept = 0;
			virtual size_t GetCsmAreaGuid()const noexcept = 0;
		protected:
			JCsmOption& GetCsmOptionRef()const noexcept;
		protected:
			void SetCsmOption(const JCsmOption& newOption)noexcept;   
		protected: 
			void CsmUpdate(const DirectX::XMMATRIX lightView,
				const DirectX::BoundingBox& sceneBBoxW,			//world bbox 
				const size_t mapSize);
		private:
			void CalculateShadowMap(const DirectX::XMMATRIX lightView,
				const DirectX::BoundingBox& sceneBBoxW,			//world bbox
				const DirectX::BoundingFrustum& camFrustumW,	//world frustum
				const size_t mapSize,
				const uint targetIndex);
		protected:
			virtual void NotifyAddCsmTarget(const uint index) = 0;
			virtual void NotifyPopCsmTarget(const uint index) = 0;
			virtual void NotifyCsmTargetZero() = 0;
		private: 
			//managed by JCsmManager
			JUserPtr<JCsmTargetInfo> CreateTargetInfo(); 
			bool DestroyTargetInfo(const int index);
			void DestroyAllTargetInfo();
		protected:
			bool RegisterCsmHandlerface();
			bool DeRegisterCsmHandlerface();
		protected:
			virtual ~JCsmHandlerInterface() = default;
		};
		using JCsmHandlerPointer = Core::JTypeImplInterfacePointer<JCsmHandlerInterface>;
	}
}