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


#include"JGraphicInfo.h"
#include"../Core/File/JFileIOHelper.h"
#include"../Core/File/JFileConstant.h"
#include"../Application/Project/JApplicationProject.h"

namespace JinEngine::Graphic
{ 
	JGraphicInfo::FrameResourceInfo::FrameResourceInfo()
	{
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		{
			count[i] = 0;
			capacity[i] = minCapacity;
		} 
	}
	uint JGraphicInfo::FrameResourceInfo::GetCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return count[(uint)type];
	}
	uint JGraphicInfo::FrameResourceInfo::GetCapacity(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept
	{
		return capacity[(uint)type];
	}
	uint JGraphicInfo::FrameResourceInfo::GetLocalLightCapacity()const noexcept
	{
		return count[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::POINT_LIGHT] +
			count[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::SPOT_LIGHT] +
			count[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::RECT_LIGHT];
	} 

	JGraphicInfo::GraphicResourceInfo::GraphicResourceInfo()
	{
		for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		{
			count[i] = 0;
			border[i] = minCapacity;
		}
	}
	uint JGraphicInfo::GraphicResourceInfo::GetCount(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept
	{
		return count[(uint)type];
	}
	uint JGraphicInfo::GraphicResourceInfo::GetBorder(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept
	{
		return border[(uint)type];
	}

	void JGraphicInfo::Load()
	{
		JFileIOTool tool;
		const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicInfo.txt");
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON, JFileIOTool::BEGIN_OPTION_JSON_TRY_LOAD_DATA))
			return;

		tool.PushExistStack("--FrameInfo--");
		//for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
		//	JFileIOHelper::LoadAtomicData(tool, frame.capacity[i], "UploadCapacity:" + Core::GetName((J_FRAME_RESOURCE_UPLOAD_TYPE)i));
		tool.PopStack();

		tool.PushExistStack("--ResourceInfo--");
		//for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		//	JFileIOHelper::LoadAtomicData(tool, resource.border[i], "UploadBorder:" + Core::GetName((J_GRAPHIC_RESOURCE_TYPE)i));
		tool.PopStack();
		tool.Close();  
	}
	void JGraphicInfo::Store()
	{
		JFileIOTool tool;
		const std::wstring path = Core::JFileConstant::MakeFilePath(JApplicationProject::ConfigPath(), L"GraphicInfo.txt");
		if (!tool.Begin(path, JFileIOTool::TYPE::JSON))
			return;

		tool.PushMapMember("--FrameInfo--");
		for (uint i = 0; i < (uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT; ++i)
			JFileIOHelper::StoreAtomicData(tool, frame.capacity[i], "UploadCapacity:" + Core::GetName((J_FRAME_RESOURCE_UPLOAD_TYPE)i));
		tool.PopStack();

		tool.PushMapMember("--ResourceInfo--");
		//for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
		//	JFileIOHelper::StoreAtomicData(tool, resource.border[i], "UploadBorder:" + Core::GetName((J_GRAPHIC_RESOURCE_TYPE)i));
		tool.PopStack();
		tool.Close(JFileIOTool::CLOSE_OPTION_JSON_STORE_DATA);
	}
}