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
#include <wrl.h>  
#include <string>
#include <memory>    
#include"../../../Core/JCoreEssential.h"  
#include "../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"   

#ifndef ReleaseCom
#define ReleaseCom(x) { if(x){ x->Release(); x = 0; } }
#endif

namespace JinEngine
{  
	class JDx12Utility
	{
	public: 	  
		static Microsoft::WRL::ComPtr<ID3DBlob> LoadBinary(const std::wstring& filename); 
	public:
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBeforeT, D3D12_RESOURCE_STATES stateBeforeF, const bool condition, D3D12_RESOURCE_STATES stateAfter);
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource* pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfterT, D3D12_RESOURCE_STATES stateAfterF, const bool condition);
		template<size_t ...Is>
		static void ResourceTransition(_In_ ID3D12GraphicsCommandList* commandList, _In_ ID3D12Resource** pResource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter, std::index_sequence<Is...>)
		{
			(ResourceTransition(commandList, pResource[Is], stateBefore, stateAfter), ...);
		}
	};
}