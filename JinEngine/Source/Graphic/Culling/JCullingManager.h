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
#include"../JGraphicSubClassInterface.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/Reflection/JReflection.h"  
  
namespace JinEngine
{
	namespace Graphic
	{ 
		class JCullingResultHolder;
		class JGraphicDevice; 

		class JCullingCreationDesc
		{
		public:
			size_t capacity = 0;
			J_CULLING_TARGET target;
			uint currFrameIndex = 0;
			bool useGpu = true;
		};
		//Manage culling resource(buffer)
		class JCullingManager : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
		private:
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JCullingManager)
		private: 
			std::vector<JOwnerPtr<JCullingInfo>> cullingInfo[(uint)J_CULLING_TYPE::COUNT];
		public:
			~JCullingManager();
		public:
			uint GetCullingInfoCount(const J_CULLING_TYPE type)const noexcept;
			JUserPtr<JCullingInfo> GetCullingInfo(const J_CULLING_TYPE type, const uint index)const noexcept;   
			std::vector<JUserPtr<JCullingInfo>> GetCullingInfoVec(const J_CULLING_TYPE type)const noexcept;
		protected:
			JCullingResultHolder* GetHolder(JCullingInfo* info)const noexcept;
			std::vector<JOwnerPtr<JCullingInfo>>& GetCullinginfoRefVec(const J_CULLING_TYPE type)noexcept;
		public:
			void ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity);
			void ReBuildBuffer(const J_CULLING_TYPE type, JGraphicDevice* device, const size_t capacity, const J_CULLING_TARGET bufferTarget);
			virtual void StuffClearValue(JGraphicDevice* device, const J_CULLING_TYPE type) = 0;
		protected:
			virtual void ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index) = 0;
		public:
			virtual JUserPtr<JCullingInfo> CreateFrsutumData(JGraphicDevice* device, const JCullingCreationDesc& desc) = 0;
			virtual JUserPtr<JCullingInfo> CreateHzbOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc) = 0;
			virtual JUserPtr<JCullingInfo> CreateHdOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc) = 0; 
		protected:
			JUserPtr<JCullingInfo> PostCreation(JOwnerPtr<JCullingInfo>&& newInfo, const J_CULLING_TYPE type);
		public:
			bool DestroyCullingData(JCullingInfo* info, JGraphicDevice* device);
		protected:
			virtual void PostDestruction(const J_CULLING_TYPE type, const uint index) = 0;
		public:
			virtual bool TryStreamOutCullingBuffer(JCullingInfo* info, const std::string& logName) = 0;
		public:
			virtual void Clear();
		private:
			void ClearResource();
		public:
			static void RegisterTypeData();
		};
	}
}