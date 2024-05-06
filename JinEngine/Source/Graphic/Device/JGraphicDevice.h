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
#include"JGraphicDeviceType.h"
#include"../JGraphicSubClassInterface.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
		class JGraphicPublicCommnad;

		//manage graphic device
		class JGraphicDevice : public JGraphicSubClassInterface
		{ 
		private:
			friend class JGraphic;
		public: 
			struct RefSet
			{
			public:
				virtual ~RefSet() = default;
			public:
				virtual J_GRAPHIC_DEVICE_TYPE GetDeviceType() const noexcept = 0;
			};
		public:
			JGraphicDevice() = default;
			virtual ~JGraphicDevice() = default;
		public:
			virtual bool CreateDeviceObject() = 0;
			virtual bool CreateRefResourceObject(const JGraphicDeviceInitSet& dataSet) = 0;
		public:
			virtual void Clear()noexcept = 0;
		public:
			virtual J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept = 0;
			virtual std::unique_ptr<RefSet> GetDeviceRefSet()const noexcept = 0;
			virtual GraphicFence GetFenceValue()const noexcept = 0;
			virtual void GetLastDeviceError(_Out_ std::wstring& errorCode, _Out_ std::wstring& errorMsg) = 0;
		public:
			//return true if device support public command
			virtual bool IsSupportPublicCommand()const noexcept = 0;
			//return true if PublicCommand is activated
			virtual bool IsPublicCommandStared()const noexcept = 0;
			virtual bool IsRaytracingSupported()const noexcept = 0;
			//return true if PublicCommand is deactivated
			virtual bool CanStartPublicCommand()const noexcept = 0;
			virtual bool CanBuildGpuAccelerator()const noexcept = 0;
		public:
			//if !IsSupportPublicCommand just return false
			virtual void StartPublicCommand() = 0;
			//if !IsSupportPublicCommand just return false
			virtual void EndPublicCommand() = 0;
			virtual void FlushCommandQueue() = 0;  
		public:
			//execute FlushCommandQueue and StartPublicCommand if CanStartPublicCommand
			void StartPublicCommandSet(bool& startCommandThisFunc);
			//execute EndPublicCommand and FlushCommandQueue if startCommandThisFunc == true
			void EndPublicCommandSet(const bool startCommandThisFunc, const bool canRestartImmdetely = false);
			void ReStartPublicCommandSet();
		public:
			virtual void UpdateWait(const GraphicFence frameFence) = 0;
		public:
			virtual void ResizeWindow(const JGraphicDeviceInitSet& dataSet) = 0;
			virtual void NotifyChangedBackBufferFormat(const JGraphicDeviceInitSet& dataSet) = 0;
		};
	}
}