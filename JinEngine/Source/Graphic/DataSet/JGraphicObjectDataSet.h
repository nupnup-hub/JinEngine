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
#include"../../Core/JCoreEssential.h"
#include"../../Core/Storage/JStorage.h"
#include"../../Object/GraphicRule/JGraphicModuleManagedDataFrame.h"
#include"../Accelerator/JGpuAcceleratorInterface.h"
#include"../Culling/JCullingInterface.h" 
#include"../ShadowMap/JCsmHandlerInterface.h"
#include"../ShadowMap/JCsmTargetInterface.h"
#include"../FrameResource/JFrameUpdateInterface.h"
#include"../GraphicResource/JGraphicResourceInterface.h"

namespace JinEngine
{ 
	namespace Graphic
	{ 
		/*
		* struct JGraphicObjectDataCreationHelperSet : public Core::JValidInterface
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* graphicResourceM;
			JCullingManager* cullingM;
		public:
			JGraphicObjectDataCreationHelperSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, JCullingManager* cullingM);
		}; 
		*/ 
		class JGraphicObjectDataSetBase : public JGraphicModuleManagedDataFrame
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataSetBase)
		public:
			template<typename Type>
			struct TypeDetermine
			{
			public:
				static constexpr bool csmHandler = std::is_base_of_v<JCsmHandlerInterface, Type>;
				static constexpr bool csmTarget = std::is_base_of_v<JCsmTargetInterface, Type>;
				static constexpr bool culling = std::is_base_of_v<JCullingInterface, Type>;
				static constexpr bool frame = std::is_base_of_v<JFrameUpdateInterface, Type>;
				static constexpr bool gpuAcc = std::is_base_of_v<JGpuAcceleratorInterface, Type>;
				static constexpr bool graphicResource = std::is_base_of_v<JGraphicResourceInterface, Type>;
			public:
				static constexpr bool isValid = csmHandler || csmTarget || culling || frame || gpuAcc || graphicResource;
			};
		public:
			JGraphicObjectDataSetBase(const JUserPtr<JObject>& object);
		private:
			JCsmHandleUserInterface* GetCsmHandleUserInterface()const noexcept final;
			JCsmTargetUserInterface* GetCsmTargetUserInterface()const noexcept final;
			JCullingUserInterface* GetCullingUserInterface()const noexcept final;
			JFrameUpdateUserInterface* GetFrameUpdateUserInterface()const noexcept final;
			JGpuAcceleratorUserInterface* GetGpuAcceleratorUserInterface()const noexcept final;
			JGraphicResourceUserInterface* GetGraphicResourceUserInterface()const noexcept final; 
		public:
			virtual JCsmHandlerInterface* GetCsmHandleInterface()const noexcept = 0;
			virtual JCsmTargetInterface* GetCsmTargetInterface()const noexcept = 0;
			virtual JCullingInterface* GetCullingInterface()const noexcept = 0;
			virtual JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept = 0;
			virtual JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept = 0;
			virtual JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept = 0;
		};  
		   
 

		template<typename Type>
		class JGraphicObjectDataOneSocket : public JGraphicObjectDataSetBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataOneSocket) 
		public:
			using TypeDetermine = TypeDetermine<Type>;
		private:
			static constexpr bool isValid = TypeDetermine::isValid;
			static_assert(isValid, "Invalid template parameter");
		private:
			std::unique_ptr<Type> firstInterface;
		public:
			JGraphicObjectDataOneSocket(const JUserPtr<JObject>& object, std::unique_ptr<Type>&& firstInterface)
				:JGraphicObjectDataSetBase(object), firstInterface(std::move(firstInterface))
			{}
		public:
			JCsmHandlerInterface* GetCsmHandleInterface()const noexcept final
			{
				if constexpr (TypeDetermine::csmHandler)
					return firstInterface.get();
				else
					return nullptr;
			}
			JCsmTargetInterface* GetCsmTargetInterface()const noexcept final
			{
				if constexpr (TypeDetermine::csmTarget)
					return firstInterface.get();
				else
					return nullptr;
			}
			JCullingInterface* GetCullingInterface()const noexcept final
			{
				if constexpr (TypeDetermine::culling)
					return firstInterface.get();
				else
					return nullptr;
			}
			JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept final
			{
				if constexpr (TypeDetermine::frame)
					return firstInterface.get();
				else
					return nullptr;
			}
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept final
			{
				if constexpr (TypeDetermine::gpuAcc)
					return firstInterface.get();
				else
					return nullptr;
			}
			JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept final
			{
				if constexpr (TypeDetermine::graphicResource)
					return firstInterface.get();
				else
					return nullptr;
			}
		}; 
		  
		template<typename FirstType, typename SecondType>
		class JGraphicObjectDataDoubleSocket : public JGraphicObjectDataSetBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataDoubleSocket)
		public:
			using FirstTypeDetermine = TypeDetermine<FirstType>;
			using SecondTypeDetermine = TypeDetermine<SecondType>;
		private:
			static constexpr bool isValid = FirstTypeDetermine::isValid || SecondTypeDetermine::isValid;
			static_assert(isValid, "Invalid template parameter");
		private:
			std::unique_ptr<FirstType> firstInterface;
			std::unique_ptr<SecondType> secondInterface;
		public:
			JGraphicObjectDataDoubleSocket(const JUserPtr<JObject>& object, std::unique_ptr<FirstType>&& firstInterface, std::unique_ptr<SecondType>&& secondInterface)
				:JGraphicObjectDataSetBase(object), firstInterface(std::move(firstInterface)), secondInterface(std::move(secondInterface))
			{}
		public:
			JCsmHandlerInterface* GetCsmHandleInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmHandler)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmHandler)
					return secondInterface.get();
				else
					return nullptr;
			}
			JCsmTargetInterface* GetCsmTargetInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmTarget)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmTarget)
					return secondInterface.get();
				else
					return nullptr;
			}
			JCullingInterface* GetCullingInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::culling)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::culling)
					return secondInterface.get();
				else
					return nullptr; 
			}
			JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::frame)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::frame)
					return secondInterface.get();
				else
					return nullptr; 
			}
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::gpuAcc)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::gpuAcc)
					return secondInterface.get();
				else
					return nullptr;
			}
			JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::graphicResource)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::graphicResource)
					return secondInterface.get();
				else
					return nullptr;
			}
		}; 
		 
		template<typename FirstType, typename SecondType, typename ThirdType>
		class JGraphicObjectDataTripleSocket : public JGraphicObjectDataSetBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataTripleSocket)
		public:
			using FirstTypeDetermine = TypeDetermine<FirstType>;
			using SecondTypeDetermine = TypeDetermine<SecondType>;
			using ThirdTypeDetermine = TypeDetermine<ThirdType>;
		private:
			static constexpr bool isValid = FirstTypeDetermine::isValid || SecondTypeDetermine::isValid || ThirdTypeDetermine::isValid;
			static_assert(isValid, "Invalid template parameter");
		private:
			std::unique_ptr<FirstType> firstInterface;
			std::unique_ptr<SecondType> secondInterface;
			std::unique_ptr<ThirdType> thirdInterface;
		public:
			JGraphicObjectDataTripleSocket(const JUserPtr<JObject>& object,
				std::unique_ptr<FirstType>&& firstInterface, 
				std::unique_ptr<SecondType>&& secondInterface,
				std::unique_ptr<ThirdType>&& thirdInterface)
				:JGraphicObjectDataSetBase(object), 
				firstInterface(std::move(firstInterface)),
				secondInterface(std::move(secondInterface)),
				thirdInterface(std::move(thirdInterface))
			{}
		public:
			JCsmHandlerInterface* GetCsmHandleInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmHandler)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmHandler)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::csmHandler)
					return thirdInterface.get();
				else
					return nullptr;
			}
			JCsmTargetInterface* GetCsmTargetInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmTarget)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmTarget)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::csmTarget)
					return thirdInterface.get();
				else
					return nullptr;
			}
			JCullingInterface* GetCullingInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::culling)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::culling)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::culling)
					return thirdInterface.get();
				else
					return nullptr;
			}
			JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::frame)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::frame)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::frame)
					return thirdInterface.get();
				else
					return nullptr;
			}
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::gpuAcc)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::gpuAcc)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::gpuAcc)
					return thirdInterface.get();
				else
					return nullptr;
			}
			JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::graphicResource)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::graphicResource)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::graphicResource)
					return thirdInterface.get();
				else
					return nullptr;
			}
		};
 
		template<typename FirstType, typename SecondType, typename ThirdType, typename ForthType>
		class JGraphicObjectDataQuadrupleSocket : public JGraphicObjectDataSetBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataQuadrupleSocket)
		public:
			using FirstTypeDetermine = TypeDetermine<FirstType>;
			using SecondTypeDetermine = TypeDetermine<SecondType>;
			using ThirdTypeDetermine = TypeDetermine<ThirdType>;
			using ForthTypeDetermine = TypeDetermine<ForthType>;
		private:
			static constexpr bool isValid = FirstTypeDetermine::isValid || SecondTypeDetermine::isValid || ThirdTypeDetermine::isValid || ForthTypeDetermine::isValid;
			static_assert(isValid, "Invalid template parameter");
		private:
			std::unique_ptr<FirstType> firstInterface;
			std::unique_ptr<SecondType> secondInterface;
			std::unique_ptr<ThirdType> thirdInterface;
			std::unique_ptr<ForthType> forthInterface;
		public:
			JGraphicObjectDataQuadrupleSocket(const JUserPtr<JObject>& object,
				std::unique_ptr<FirstType>&& firstInterface,
				std::unique_ptr<SecondType>&& secondInterface,
				std::unique_ptr<ThirdType>&& thirdInterface,
				std::unique_ptr<ForthType>&& forthInterface)
				:JGraphicObjectDataSetBase(object),
				firstInterface(std::move(firstInterface)),
				secondInterface(std::move(secondInterface)),
				thirdInterface(std::move(thirdInterface)),
				forthInterface(std::move(forthInterface))
			{}
		public:
			JCsmHandlerInterface* GetCsmHandleInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmHandler)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmHandler)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::csmHandler)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::csmHandler)
					return forthInterface.get();
				else
					return nullptr;
			}
			JCsmTargetInterface* GetCsmTargetInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::csmTarget)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::csmTarget)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::csmTarget)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::csmTarget)
					return forthInterface.get();
				else
					return nullptr;
			}
			JCullingInterface* GetCullingInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::culling)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::culling)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::culling)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::culling)
					return forthInterface.get();
				else
					return nullptr;
			}
			JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::frame)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::frame)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::frame)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::frame)
					return forthInterface.get();
				else
					return nullptr;
			}
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::gpuAcc)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::gpuAcc)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::gpuAcc)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::gpuAcc)
					return forthInterface.get();
				else
					return nullptr;
			}
			JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept final
			{
				if constexpr (FirstTypeDetermine::graphicResource)
					return firstInterface.get();
				else if constexpr (SecondTypeDetermine::graphicResource)
					return secondInterface.get();
				else if constexpr (ThirdTypeDetermine::graphicResource)
					return thirdInterface.get();
				else if constexpr (ForthTypeDetermine::graphicResource)
					return forthInterface.get();
				else
					return nullptr;
			}
		};
		/*	
		class JGraphicObjectDataSetAllInOne : public JGraphicObjectDataSetBase
		{
			REGISTER_CLASS_USE_ALLOCATOR(JGraphicObjectDataSetAllInOne)
		public:
			std::unique_ptr<JCullingInterface> cullingInterface;
			std::unique_ptr<JFrameUpdateInterface> frameInterface;
			std::unique_ptr<JGpuAcceleratorInterface> gpuAcceleratorInterface;
			std::unique_ptr<JGraphicResourceInterface> graphicResourceInterface;
		public:
			JGraphicObjectDataSetAllInOne(const JUserPtr<JObject>& object,
				std::unique_ptr<JCullingInterface>&& cullingInterface,
				std::unique_ptr<JFrameUpdateInterface>&& frameInterface,
				std::unique_ptr<JGpuAcceleratorInterface>&& gpuAcceleratorInterface,
				std::unique_ptr<JGraphicResourceInterface>&& graphicResourceInterface);
		public:
			JCsmHandlerInterface* GetCsmHandleInterface()const noexcept final;
			JCsmTargetInterface* GetCsmTargetInterface()const noexcept final;
			JCullingInterface* GetCullingInterface()const noexcept final;
			JFrameUpdateInterface* GetFrameUpdateInterface()const noexcept final;
			JGpuAcceleratorInterface* GetGpuAcceleratorInterface()const noexcept final;
			JGraphicResourceInterface* GetGraphicResourceInterface()const noexcept final;
		};
		*/

		struct JObjectDataSetMetadata
		{ 
		private:
			static constexpr int invalidType = invalidIndex;
		public:
			size_t typeGuid = 0;
		public:
			//type hint
			int componentTypeValue = invalidType;				//-1 if not component 	
			int resourceTypeValue = invalidType;				//-1 if not resource 	
		public:
			//feature
			bool isSupportedCulling = false;
			bool isSupportedFrameResourceUpload = false;
			bool isSupportedFrameDirty = false;
			bool isSupportedGpuAccelerator = false;
			bool isSupportedGraphicResource = false;
		public:
			bool IsComponentType()const noexcept;
			bool IsResourceType()const noexcept;
		};

		using ObjectDataSetVec = Core::JVectorStorage<JOwnerPtr<JGraphicObjectDataSetBase>>;
	}
}