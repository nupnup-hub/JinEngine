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


#include"JCsmHandlerInterface.h"
#include"JCsmTargetInfo.h"  
#include"../../Core/Utility/JCommonUtility.h" 
 
using namespace DirectX;
namespace JinEngine::Graphic
{
	JCsmOption JCsmHandlerInterface::GetOption()const noexcept
	{
		return info != nullptr ? info->GetOption() : JCsmOption();
	}
	uint JCsmHandlerInterface::GetTargetCount()const noexcept
	{
		return info != nullptr ? info->GetTargetCount() : 0;
	}
	const JCsmComputeResult& JCsmHandlerInterface::GetComputeResult(const uint index)const noexcept
	{
		static JCsmComputeResult emptyResult;
		return info != nullptr ? info->GetComputeResult(index) : emptyResult;
	}
	void JCsmHandlerInterface::SetOption(const JCsmOption& newOption)noexcept
	{
		if (info == nullptr)
			return;

		info->SetOption(newOption);
	}
	void JCsmHandlerInterface::Update(DirectX::XMMATRIX lightView,
		const DirectX::BoundingBox& sceneBBoxW,
		const size_t mapSize)
	{
		if (info == nullptr)
			return;

		info->Update(lightView, sceneBBoxW, mapSize);
	} 
}