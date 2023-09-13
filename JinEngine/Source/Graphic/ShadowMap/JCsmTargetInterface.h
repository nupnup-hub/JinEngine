#pragma once
#include"JCsmType.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Reflection/JTypeImplBase.h"
#include<unordered_map>

namespace JinEngine
{
	namespace Graphic
	{
		class JCsmTargetInfo;  
		class JCsmManager;
		class JCsmTargetInterface : public Core::JTypeImplInterfacePointerHolder<JCsmTargetInterface>
		{
		private:
			friend class JCsmManager;
		private:
			int targetIndex = -1;
			std::vector<JUserPtr<JCsmTargetInfo>> infoVec; 
		protected:
			//managed by JCsmManager
			void AddCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info);
			void RemoveCsmTargetInfo(const JUserPtr<JCsmTargetInfo>& info);
		protected:
			void TrySetGetFrustumPtr()noexcept;
		public:
			/**
			* @brief aligned by registed time
			* @return target index in Csm resource handler if info == nullptr return -1
			*/
			int GetCsmTargetIndex()const noexcept;
			virtual size_t GetCsmTargetGuid()const noexcept = 0;
			virtual size_t GetCsmAreaGuid()const noexcept = 0;
			virtual DirectX::BoundingFrustum GetBoundingFrustum()const noexcept = 0;
		private:
			JUserPtr<JCsmTargetInfo> GetTargetInfo(const size_t handlerGuid)const noexcept;
			JUserPtr<JCsmTargetInfo> GetTargetInfo(const size_t handlerGuid, _Out_ int& index)const noexcept;
		public:
			bool HasCsmTargetInfo()const noexcept;
		protected:
			bool RegisterCsmTargetInterface();
			bool DeRegisterCsmTargetInterface();
		protected: 
			virtual ~JCsmTargetInterface() = default;
		};
		using JCsmTargetInterfacePointer = Core::JTypeImplInterfacePointer<JCsmTargetInterface>;
	}
}