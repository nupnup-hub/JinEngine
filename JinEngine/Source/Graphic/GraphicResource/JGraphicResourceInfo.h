#pragma once
#include"../Device/JGraphicDeviceUser.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h"   
#include"../../Core/Reflection/JReflection.h" 
#include"JGraphicResourceType.h"  
 
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
		public:
			struct ResourceViewInfo
			{
			public:	 
				int stIndex = -1;
				uint count = 0;
			};  
			struct OptionResourceViewInfo
			{
			public:
				ResourceViewInfo viewInfo[(int)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT][(int)J_GRAPHIC_BIND_TYPE::COUNT];
			};
		private:
			const J_GRAPHIC_RESOURCE_TYPE graphicResourceType; 
			int resourceArrayIndex = -1;   
		private:
			ResourceViewInfo viewInfo[(int)J_GRAPHIC_BIND_TYPE::COUNT];
			std::unique_ptr<OptionResourceViewInfo> optionalInfo; 
		private:
			J_GRAPHIC_MIP_MAP_TYPE mipMapType = J_GRAPHIC_MIP_MAP_TYPE::GRAPHIC_API_DEFAULT; 
		public:
			J_GRAPHIC_RESOURCE_TYPE GetGraphicResourceType()const noexcept;
			int GetArrayIndex()const noexcept; 
			int GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			int GetOptionHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept;
			int GetMinValidHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;			//consider info & option
			uint GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			uint GetOptionViewCount(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept;
			virtual uint GetWidth()const noexcept = 0;
			virtual uint GetHeight()const noexcept = 0; 
			virtual uint GetElementCount()const noexcept = 0;		//buffer byte size  / element size
			virtual uint GetElementSize()const noexcept = 0;		//buffer byte size  / element size
			virtual uint GetMipmapCount()const noexcept = 0;
			virtual J_GRAPHIC_RESOURCE_FORMAT GetFormat()const noexcept = 0;
			virtual JVector2<uint> GetResourceSize()const noexcept = 0;
			virtual ResourceHandle GetResourceGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const uint bIndex = 0)const noexcept = 0;
			virtual ResourceHandle GetResourceOptionGpuHandle(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex = 0)const noexcept = 0;
			J_GRAPHIC_MIP_MAP_TYPE GetMipmapType()const noexcept;
		public:
			virtual void SetPrivateName(const std::wstring& name)noexcept = 0;
			virtual void SetPrivateOptionName(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const std::wstring& name)noexcept = 0;
			void SetArrayIndex(const int newValue)noexcept; 
			void SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue);
			void SetHeapOptionIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int newValue);
			void SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue);
			void SetOptionViewCount(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint newValue);
			void SetMipmapType(J_GRAPHIC_MIP_MAP_TYPE newMipmapType)noexcept;
		public:
			bool HasView(const J_GRAPHIC_BIND_TYPE bindType)const noexcept;
			virtual bool HasOption(const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept = 0; 
		public: 
			static bool Destroy(JGraphicResourceInfo* info);  
		protected:
			void TryCreateOptionViewInfo();
			void DestroyOptionViewInfo();
		protected:
			JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType);
			virtual ~JGraphicResourceInfo();
		};
	}
}