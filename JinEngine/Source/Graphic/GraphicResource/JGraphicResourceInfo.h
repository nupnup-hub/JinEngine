#pragma once
#include"../Device/JGraphicDeviceUser.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Reflection/JReflection.h" 
#include"JGraphicResourceType.h" 
#include<memory>
#include<string> 
 
namespace JinEngine
{
	namespace Graphic
	{ 		
		/*
		* JGraphicResourceManager에서만 생성가능한 Info
		* 자원을 JGraphicResourceManager 에서 할당받아 Graphic Resource에 참조를 소유하는 Class에서 참조한다
		* 항상 유효한 객체이다
		*/  
		class JGraphicResourceInfo : public JGraphicDeviceUser
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicResourceInfo) 
		private:
			struct ResourceViewInfo
			{
			public:	 
				int stIndex = -1;
				uint count = 0;
			};  
		private:
			const J_GRAPHIC_RESOURCE_TYPE graphicResourceType; 
			int resourceArrayIndex = -1;  
		private:
			ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
		public:
			J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			int GetArrayIndex()const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			uint GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			virtual uint GetWidth()const noexcept = 0;
			virtual uint GetHeight()const noexcept = 0; 
			virtual ResourceHandle GetResourceHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex = 0)const noexcept = 0;
		public:
			virtual void SetPrivateName(const std::wstring& name)noexcept = 0;
			void SetArrayIndex(const int newValue)noexcept;
			void SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue);
			void SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue);
		public:
			bool HasView(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
		public: 
			static bool Destroy(JGraphicResourceInfo* info);  
		protected:
			JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType);
			virtual ~JGraphicResourceInfo();
		};
	}
}