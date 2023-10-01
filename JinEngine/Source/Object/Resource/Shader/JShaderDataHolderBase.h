#pragma once
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Math/JVector.h"
#include"JShaderFunctionEnum.h"
#include"JShaderCondition.h"
#include<string>
namespace JinEngine
{
	//Is Same as {J_DEVICE_TYPE, ...}
	using DeviceIdentification = uint;
	class JGraphicShaderDataHolderBase
	{ 
	public:
		JGraphicShaderDataHolderBase() = default;
		virtual ~JGraphicShaderDataHolderBase() = default;
	public:
		virtual DeviceIdentification GetDeviceIdentification()const noexcept = 0;
	public:	
		//check convertable device
		virtual bool IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept = 0;
		virtual bool HasExtra(const J_GRAPHIC_SHADER_EXTRA_FUNCTION type)const noexcept = 0;
	public:
		virtual void Clear() = 0;
	};

	class JComputeShaderDataHolderBase
	{
	public:
		struct DispatchInfo
		{
		public:
			JVector3<uint> groupDim = JVector3<uint>::Zero();
			JVector3<uint> threadDim = JVector3<uint>::Zero();
			uint taskOriCount = 0;
		public:
			uint GetTotalThreadCount()const noexcept;
		};
	public:
		DispatchInfo dispatchInfo;													//Variable
	public:
		JComputeShaderDataHolderBase() = default;
		virtual ~JComputeShaderDataHolderBase() = default;
	public:
		virtual DeviceIdentification GetDeviceIdentification()const noexcept = 0;
	public:
		virtual bool IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept = 0;
	public:
		virtual void Clear() = 0;
	};

	struct JMacroSet
	{
	public:
		std::string name;
		std::string value;
	};
	struct JGraphicShaderInitData
	{
	public:
		std::vector<JMacroSet> macro[SHADER_VERTEX_COUNT]; 
		J_GRAPHIC_SHADER_FUNCTION gFunctionFlag;
		J_SHADER_VERTEX_LAYOUT layoutType;
		JShaderCondition condition; 
	};
	struct JComputeShaderInitData
	{
	public:
		std::vector<JMacroSet> macro; 
		J_COMPUTE_SHADER_FUNCTION cFunctionFlag;
		JComputeShaderDataHolderBase::DispatchInfo dispatchInfo;
	};
}