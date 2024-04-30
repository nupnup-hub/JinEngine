#include"JShaderType.h"
#include"../../Core/File/JFileIOHelper.h"
#include<string>

#define THREAD_DIM_X_SYMBOL L"DIMX"
#define THREAD_DIM_Y_SYMBOL L"DIMY"
#define THREAD_DIM_Z_SYMBOL L"DIMZ"

namespace JinEngine
{
	std::wstring JShaderType::ConvertToName(const J_GRAPHIC_SHADER_FUNCTION funcFlag, const size_t subPsoID)
	{
		return L"Gs" + std::to_wstring(funcFlag) + L"_" + std::to_wstring(subPsoID);
	}
	std::wstring JShaderType::ConvertToName(const J_COMPUTE_SHADER_FUNCTION funcFlag)
	{
		return L"Cs" + std::to_wstring((int)funcFlag);
	}
	J_GRAPHIC_SHADER_VERTEX_LAYOUT JShaderType::ConvertToVertexLayout(const Core::J_MESHGEOMETRY_TYPE type)
	{
		return type == Core::J_MESHGEOMETRY_TYPE::STATIC ? J_GRAPHIC_SHADER_VERTEX_LAYOUT::STATIC : J_GRAPHIC_SHADER_VERTEX_LAYOUT::SKINNED;
	}

	uint JDispatchInfo::GetTotalThreadCount()const noexcept
	{
		return (groupDim.x * groupDim.y * groupDim.z) * (threadDim.x * threadDim.y * threadDim.z);
	}

	JCompileInfo::JCompileInfo(const std::wstring& fileName, const std::wstring& functionName)
		:filePath(JApplicationEngine::ShaderPath() + L"\\" + fileName), functionName(functionName) {}
	bool JCompileInfo::IsValid()const noexcept
	{
		return !functionName.empty() && JFileIOHelper::HasFile(filePath);
	}

	void JComputeShaderInitData::PushThreadDimensionMacro()
	{
		macro.push_back({ THREAD_DIM_X_SYMBOL, std::to_wstring(dispatchInfo.threadDim.x) });
		macro.push_back({ THREAD_DIM_Y_SYMBOL, std::to_wstring(dispatchInfo.threadDim.y) });
		macro.push_back({ THREAD_DIM_Z_SYMBOL, std::to_wstring(dispatchInfo.threadDim.z) });
	}
	void JComputeShaderInitData::CalThreadAndGroupDim(const Core::JHardwareInfo::GpuInfo& info, const uint width, const uint height)
	{
		dispatchInfo.groupDim.x = (uint)std::ceil((float)width / float(info.maxThreadsDim.x));
		dispatchInfo.groupDim.y = height < info.maxThreadsDim.y ? height : info.maxThreadsDim.y;

		dispatchInfo.threadDim.x = width < info.maxThreadsDim.x ? width : info.maxThreadsDim.x;
		dispatchInfo.threadDim.y = (uint)std::ceil((float)height / float(info.maxGridDim.y));
	}
}