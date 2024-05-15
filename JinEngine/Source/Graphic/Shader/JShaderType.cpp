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


#include"JShaderType.h"
#include"../../Core/File/JFileIOHelper.h" 

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

	JCompileInfo::JCompileInfo(const std::wstring& relativePath, const std::wstring& functionName)
		:filePath(JApplicationEngine::ShaderPath() + L"\\" + relativePath), functionName(functionName) {}
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