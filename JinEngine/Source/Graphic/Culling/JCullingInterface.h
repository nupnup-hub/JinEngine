#pragma once
#include"JCullingType.h"
#include"../../Core/Pointer/JOwnerPtr.h"
#include"../../Core/Reflection/JTypeImplBase.h"
#include<vector>

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
		* �ַ� impl class�� �ش� interface�� ����Ѵ�
		* interface�� info�� �����ϴ� wrapper�� culling manager�� impl���� �߰������� �Ѵ�
		* impl�� ���� JCullingInfo�� ������� ������ �ܺη� ������ �� ���⶧���̴�.
		*/
		class JCullingInterface : public Core::JTypeImplInterfacePointerHolder<JCullingInterface>
		{  
		private:
			JUserPtr<JCullingInfo> info[(uint)J_CULLING_TYPE::COUNT];
		protected:
			bool CreateFrustumCullingData();
			bool CreateHzbOccCullingData();
			bool CreateHdOccCullingData(); 
		protected:
			void DestroyCullingData(const J_CULLING_TYPE type)noexcept; 
			void DestroyAllCullingData()noexcept;
		public: 
			int GetArrayIndex(const J_CULLING_TYPE type)const noexcept;
			uint GetResultBufferSize(const J_CULLING_TYPE type)const noexcept;
			float GetUpdateFrequency(const J_CULLING_TYPE type)const noexcept;
			float GetUpdatePerObjectRate(const J_CULLING_TYPE type)const noexcept;
		public:
			void SetCulling(const J_CULLING_TYPE type, const uint index)noexcept;
			void OffCulling(const J_CULLING_TYPE type, const uint index)noexcept;
			//off user culling
			void OffCullingArray(const J_CULLING_TYPE type)noexcept; 
		public:
			bool IsCulled(const uint objectIndex)const noexcept;
			bool IsCulled(const J_CULLING_TYPE type, const uint objectIndex)const noexcept;				//culling array is ordered by render item mesh number
			bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept;
			bool HasCullingData(const J_CULLING_TYPE type)const noexcept;
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
			int GetArrayIndex(const J_CULLING_TYPE type)const noexcept;
			uint GetResultBufferSize(const J_CULLING_TYPE type)const noexcept;
			float GetUpdateFrequency(const J_CULLING_TYPE type)const noexcept;
			float GetUpdatePerObjectRate(const J_CULLING_TYPE type)const noexcept;
		public:
			void SetCulling(const J_CULLING_TYPE type, const uint index)noexcept;
			void OffCulling(const J_CULLING_TYPE type, const uint index)noexcept;
			void OffCullingArray(const J_CULLING_TYPE type)noexcept; 
		public:
			bool IsCulled(const uint objectIndex)const noexcept;											//culling array is ordered by render item mesh number
			bool IsCulled(const J_CULLING_TYPE type, const uint objectIndex)const noexcept;				//culling array is ordered by render item mesh number
			bool IsValid()const noexcept;
			bool IsUpdateEnd(const J_CULLING_TYPE type)const noexcept;
			bool HasCullingData(const J_CULLING_TYPE type)const noexcept; 												//culling array is ordered by render item mesh number
		};
	}
}