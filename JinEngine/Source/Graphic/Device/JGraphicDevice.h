#pragma once
#include"JGraphicDeviceType.h"
#include"../DataSet/JGraphicDataSet.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h"
#include<memory>
#include<string>
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphic;
		class JGraphicPublicCommnad;

		//manage graphic device
		class JGraphicDevice
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
			virtual void Clear() = 0;
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
			//return true if PublicCommand is deactivated
			virtual bool CanStartPublicCommand()const noexcept = 0;
		public:
			//if !IsSupportPublicCommand just return false
			virtual void StartPublicCommand() = 0;
			//if !IsSupportPublicCommand just return false
			virtual void EndPublicCommand() = 0;
			virtual void FlushCommandQueue() = 0; 
		public:
			virtual void UpdateWait(const GraphicFence frameFence) = 0;
		public:
			virtual void ResizeWindow(const JGraphicDeviceInitSet& refSet) = 0;
		};
	}
}