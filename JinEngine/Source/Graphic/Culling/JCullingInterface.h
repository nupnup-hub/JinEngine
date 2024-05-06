/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once
#include"JCullingType.h"
#include"JCullingInfo.h" 
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Reflection/JTypeImplBase.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JCullingInfo;
		class JGraphicMultiResourceInterface;
		class JGraphic;
		class JCullingUserInterface;
		class JCullingUpdaterInterface;

		/**
		* 주로 impl class가 해당 interface을 상속한다
		* interface는 info를 소유하는 wrapper로 culling manager와 impl간의 중간역할을 한다
		* impl이 직접 JCullingInfo를 상속하지 않은건 외부로 노출할 수 없기때문이다.
		*/
		class JCullingInterface : public Core::JTypeImplInterfacePointerHolder<JCullingInterface>
		{   
		protected:
			bool CreateFrustumCullingData(const J_CULLING_TARGET target);
			bool CreateHzbOccCullingData();
			bool CreateHdOccCullingData();  
		protected: 
			void DestroyCullingData(JUserPtr<JCullingInfo>& info)noexcept;
			virtual void DestroyAllCullingData()noexcept = 0;
		private:
			virtual void AddInfo(const JUserPtr<JCullingInfo>& newInfo) = 0;
		public: 
			int GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			uint GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			float GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept; 
		protected:
			virtual JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
		public:
			void SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept;
			void OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept;
			//off user culling
			void OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept;
		public:  
			/**
			* @return all culling type &&...
			*/
			virtual bool IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept = 0;
			bool IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept;
			virtual bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept = 0;
			bool IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			bool HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			virtual bool HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept = 0;
		protected:
			virtual bool IsValidType(const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget)const noexcept = 0;
			template<size_t... Is>
			bool IsUpdateEndT(const JUserPtr<JCullingInfo>(&ref)[(uint)J_CULLING_TYPE::COUNT], std::index_sequence<Is...>)const noexcept
			{
				return ((ref[Is] != nullptr ? ref[Is]->IsUpdateEnd() : true) && ...);
			}
			template<size_t... Is>
			bool IsUpdateEndT(const J_CULLING_TYPE  type,
				const JUserPtr<JCullingInfo>(&ref)[(uint)J_CULLING_TYPE::COUNT][(uint)J_CULLING_TARGET::COUNT],
				std::index_sequence<Is...>)const noexcept
			{
				return ((ref[(uint)type][Is] != nullptr ? ref[(uint)type][Is]->IsUpdateEnd() : true) && ...);
			}
			template<size_t... Is>
			bool IsCulledT(const uint index, const JUserPtr<JCullingInfo>(&ref)[(uint)J_CULLING_TYPE::COUNT], std::index_sequence<Is...>)const noexcept
			{ 
				return ((ref[Is] != nullptr ? ref[Is]->IsCulled(index) : false) || ...);
			}
			template<size_t... Is>
			bool IsCulledT(const J_CULLING_TARGET target,
				const uint index,
				const JUserPtr<JCullingInfo>(&ref)[(uint)J_CULLING_TYPE::COUNT][(uint)J_CULLING_TARGET::COUNT], 
				std::index_sequence<Is...>)const noexcept
			{
				return ((ref[Is][(uint)target] != nullptr ? ref[Is][(uint)target]->IsCulled(index) : false) || ...);
			}
		};

		template<J_CULLING_TYPE type, J_CULLING_TARGET target>
		class JCullingSingleHolder : public JCullingInterface
		{
		private:
			JUserPtr<JCullingInfo> info;
		protected:
			bool CreateFrustumCullingData()
			{
				return JCullingInterface::CreateFrustumCullingData(target);
			}
		public:
			void DestroyCullingData(const J_CULLING_TYPE cType)noexcept
			{
				if(type == cType)
					JCullingInterface::DestroyCullingData(info);
			}
			void DestroyAllCullingData()noexcept final
			{
				JCullingInterface::DestroyCullingData(info);
			}
		private:
			void AddInfo(const JUserPtr<JCullingInfo>& newInfo)final
			{
				info = newInfo;
			}
		public:
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final
			{
				return info;
			}
		public:
			bool IsCulled(const J_CULLING_TARGET cTarget, const uint index)const noexcept final
			{
				return info != nullptr && target == cTarget ? info->IsCulled(index) : false;
			}
			bool IsUpdateEnd(const J_CULLING_TYPE cType)const noexcept
			{
				return info != nullptr && type == cType ? info->IsUpdateEnd() : true;
			}
			bool HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final
			{
				return IsValidType(type, target) ? info == nullptr : false;
			}
		protected:
			bool IsValidType(const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget) const noexcept final
			{
				return type == cType && target == cTarget;
			}	 
		};
		template<J_CULLING_TARGET target>
		class JCullingSingleTargetHolder : public JCullingInterface
		{
		private:
			JUserPtr<JCullingInfo> info[(uint)J_CULLING_TYPE::COUNT];
		public:
			void DestroyCullingData(const J_CULLING_TYPE type)noexcept
			{
				JCullingInterface::DestroyCullingData(info[(uint)type]);
			}
			void DestroyAllCullingData()noexcept final
			{
				for(uint i = 0; i < (uint)J_CULLING_TYPE::COUNT; ++i)
					JCullingInterface::DestroyCullingData(info[i]);
			}
		private:
			void AddInfo(const JUserPtr<JCullingInfo>& newInfo)final
			{
				const J_CULLING_TYPE newType = newInfo->GetCullingType();
				info[(uint)newType] = newInfo;
			}
		public:
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final
			{
				return info[(uint)type];
			}
		public:
			//Caution
			//bool value = true ? 0 : 0 || true ? 1 : 0;  value is zero
			//bool value = (true ? 0 : 0) || ( true ? 1 : 0);  || 부터 평가됨 의도대로 표현하려면 ()필요
			//1. 0 || true -> true
			//2. true ? 0 : (1 ? 1 : 0)
			//3. value is 0 
			//type 추가시 수정필요(for or hard coding)
			bool IsCulled(const J_CULLING_TARGET cTarget, const uint index)const noexcept final
			{
				return target == cTarget ? JCullingInterface::IsCulledT(index, info, std::make_index_sequence<(uint)J_CULLING_TYPE::COUNT>()) : false;
				//return (info[(uint)J_CULLING_TYPE::FRUSTUM] != nullptr ? info[(uint)J_CULLING_TYPE::FRUSTUM]->IsCulled(index) : false) ||
				//	(info[(uint)J_CULLING_TYPE::HZB_OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::HZB_OCCLUSION]->IsCulled(index) : false) ||
				//	(info[(uint)J_CULLING_TYPE::HD_OCCLUSION] != nullptr ? info[(uint)J_CULLING_TYPE::HD_OCCLUSION]->IsCulled(index) : false);
			}
			bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept
			{
				return info[(uint)type] != nullptr ? info[(uint)type]->IsUpdateEnd() : true;
			}
			bool HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final
			{
				return IsValidType(type, target) ? info[(uint)type] == nullptr : false;
			}
		protected:
			bool IsValidType(const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget) const noexcept final
			{
				return target == cTarget;
			}
		};
		class JCullingTypePerSingleTargetHolder : public JCullingInterface
		{
		private:
			JUserPtr<JCullingInfo> info[(uint)J_CULLING_TYPE::COUNT][(uint)J_CULLING_TARGET::COUNT];
		public:
			//void DestroyCullingData(const J_CULLING_TYPE type)noexcept;
			void DestroyCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept;
			void DestroyAllCullingData()noexcept final;
		private:
			void AddInfo(const JUserPtr<JCullingInfo>& newInfo)final;
		public:
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final;
		public:
			bool IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept final;
			bool IsUpdateEnd(const J_CULLING_TYPE cType)const noexcept final;
			bool HasSpace(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept final;
		protected:
			bool IsValidType(const J_CULLING_TYPE type, const J_CULLING_TARGET target) const noexcept final;
		};

		using JCullingIntefacePointer = Core::JTypeImplInterfacePointer<JCullingInterface>;

		class JCullingUserInterface final
		{ 
		private:
			JUserPtr<JCullingIntefacePointer> cPtrWrapper;
		public: 
			JCullingUserInterface() = default;
			JCullingUserInterface(JCullingInterface* currInterface);
		public: 
			int GetArrayIndex(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			uint GetResultBufferSize(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			float GetUpdateFrequency(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept; 
		public:
			void SetCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept;
			void OffCulling(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)noexcept;
			void OffCullingArray(const J_CULLING_TYPE type, const J_CULLING_TARGET target)noexcept;
		public: 
			bool IsCulled(const J_CULLING_TARGET target, const uint index)const noexcept;	//culling array is ordered by frame index
			bool IsCulled(const J_CULLING_TYPE type, const J_CULLING_TARGET target, const uint index)const noexcept;
			bool IsValid()const noexcept; 
			bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept;
			bool IsUpdateEnd(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept;
			bool HasCullingData(const J_CULLING_TYPE type, const J_CULLING_TARGET target)const noexcept; 												//culling array is ordered by render item mesh number
		};
	}
}