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
#include"JCsmType.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Reflection/JTypeImplBase.h" 

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