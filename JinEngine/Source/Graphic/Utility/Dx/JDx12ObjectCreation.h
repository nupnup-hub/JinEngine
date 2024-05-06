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
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Exception/JExceptionMacro.h"
#include"../../../Core/Log/JLogMacro.h"
#include<d3dx12.h>

#define ROOT_INDEX_CREATOR(namespaceName, ...)	namespace namespaceName{J_MAKE_STATIC_CONSTEXPR_DATA_INDEX_LIST(__VA_ARGS__); static constexpr int rootSlotCount = J_COUNT(__VA_ARGS__);}	

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12ObjectCreation
		{  
		public:
			static void Create(ID3D12Device* device,
				const CD3DX12_ROOT_SIGNATURE_DESC* desc,
				const std::wstring& name,
				ID3D12RootSignature** outRoot);
		};

		template<uint paramCount>
		class JDx12RootSignatureBuilder
		{
		public:
			CD3DX12_DESCRIPTOR_RANGE range[paramCount];
			CD3DX12_ROOT_PARAMETER param[paramCount]; 
		private:
			uint paramIndex = 0; 
		private:
			const std::string errMsg;
		public:
			JDx12RootSignatureBuilder(const std::string& errMsg = "")
				:errMsg(errMsg)
			{}
		protected:
			uint GetParamIndex()const noexcept
			{
				return paramIndex;
			}
			std::string GetErrMsg()const noexcept
			{
				return errMsg;
			}
		public:
			void PushTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType,
				uint numDescriptors,
				uint baseShaderRegister,
				uint registerSpace = 0,
				uint offsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(paramIndex < paramCount, "Over RootSignatureBuilder range", errMsg)
				range[paramIndex].Init(rangeType, numDescriptors, baseShaderRegister, registerSpace, offsetInDescriptorsFromTableStart);		 
				param[paramIndex].InitAsDescriptorTable(1, &range[paramIndex], visibility);
				++paramIndex;
			}
			void PushShaderResource(uint shaderRegister,
				uint registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(paramIndex < paramCount, "Over RootSignatureBuilder range", errMsg)
				param[paramIndex].InitAsShaderResourceView(shaderRegister, registerSpace, visibility);
				++paramIndex;
			}
			void PushUnorderedAccess(uint shaderRegister,
				uint registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(paramIndex < paramCount, "Over RootSignatureBuilder range", errMsg)
				param[paramIndex].InitAsUnorderedAccessView(shaderRegister, registerSpace, visibility);
				++paramIndex;
			}
			void PushConstantsBuffer(uint shaderRegister,
				uint registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(paramIndex < paramCount, "Over RootSignatureBuilder range", errMsg)
				param[paramIndex].InitAsConstantBufferView(shaderRegister, registerSpace, visibility);
				++paramIndex;
			}
			void PushConstants(uint num32BitValues,
				uint shaderRegister,
				uint registerSpace = 0,
				D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(paramIndex < paramCount, "Over RootSignatureBuilder range", errMsg)
				param[paramIndex].InitAsConstants(num32BitValues, shaderRegister, registerSpace, visibility);
				++paramIndex;
			}
		public:
			void Create(ID3D12Device* device,
				const std::wstring& name,
				ID3D12RootSignature** outRoot,
				D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_NONE)
			{
				CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(paramIndex,
					param,
					0,
					nullptr,
					flag);

				JDx12ObjectCreation::Create(device, &rootSigDesc, name, outRoot);
			} 
			void Clear()
			{
				paramIndex = 0;
				for (uint i = 0; i < paramCount; ++i)
				{
					range[i] = CD3DX12_DESCRIPTOR_RANGE();
					param[i] = CD3DX12_ROOT_PARAMETER();
				}
			}
		};
  
		template<uint paramCount, uint samplerCount>
		class JDx12RootSignatureBuilder2 : public JDx12RootSignatureBuilder<paramCount>
		{
		private:
			using Parent = JDx12RootSignatureBuilder<paramCount>;
		public:
			CD3DX12_STATIC_SAMPLER_DESC sampler[samplerCount];
		private:
			uint samplerIndex = 0;
		public:
			void PushSampler(D3D12_FILTER filter,
				D3D12_TEXTURE_ADDRESS_MODE address,
				float mipLODBias = 0,
				uint maxAnisotropy = 16,
				D3D12_COMPARISON_FUNC comparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL,
				D3D12_STATIC_BORDER_COLOR borderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE,
				float minLOD = 0.0f,
				float maxLOD = D3D12_FLOAT32_MAX,
				D3D12_SHADER_VISIBILITY shaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
				uint registerSpace = 0)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(samplerIndex < samplerCount, "Over RootSignatureBuilder2 range", Parent::GetErrMsg())
				sampler[samplerIndex].Init(samplerIndex,
					filter,
					address,
					address,
					address,
					mipLODBias,
					maxAnisotropy,
					comparisonFunc,
					borderColor,
					minLOD,
					maxLOD,
					shaderVisibility,
					registerSpace);
				++samplerIndex;
			}
			void PushSampler(D3D12_FILTER filter,
				D3D12_TEXTURE_ADDRESS_MODE address,  
				D3D12_STATIC_BORDER_COLOR borderColor)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(samplerIndex < samplerCount, "Over RootSignatureBuilder2 range", Parent::GetErrMsg())
				sampler[samplerIndex].Init(samplerIndex,
					filter,
					address,
					address,
					address,
					0,
					16,
					D3D12_COMPARISON_FUNC_LESS_EQUAL,
					borderColor,
					0.0f,
					D3D12_FLOAT32_MAX,
					D3D12_SHADER_VISIBILITY_ALL,
					0);
				++samplerIndex;
			}
			void PushSampler(const CD3DX12_STATIC_SAMPLER_DESC& desc)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(samplerIndex < samplerCount, "Over RootSignatureBuilder2 range", Parent::GetErrMsg())
				sampler[samplerIndex] = desc;
				++samplerIndex;
			}
			void PushBorderSampler(D3D12_FILTER filter, D3D12_STATIC_BORDER_COLOR borderColor)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(samplerIndex < samplerCount, "Over RootSignatureBuilder2 range", Parent::GetErrMsg())
				PushSampler(filter, D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0, 16, D3D12_COMPARISON_FUNC_LESS_EQUAL, borderColor);
			}
		public:
			void Create(ID3D12Device* device,
				const std::wstring& name,
				ID3D12RootSignature** outRoot,
				D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_NONE)
			{ 
				CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(Parent::GetParamIndex(),
					Parent::param,
					samplerIndex,
					sampler,
					flag);

				JDx12ObjectCreation::Create(device, &rootSigDesc, name, outRoot);
			}
		};

		class JDx12ComputePso
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		public:
			static void Create(ID3D12Device* device,
				JDx12ComputeShaderDataHolder* holder,
				ID3D12RootSignature* root,
				D3D12_PIPELINE_STATE_FLAGS flag = D3D12_PIPELINE_STATE_FLAG_NONE);
		};

		template<uint count, uint holderPsoCount = 1>
		class JDx12ComputePsoBulder
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<holderPsoCount>;
		private:
			JDx12ComputeShaderDataHolder* holder[count];
			JCompileInfo compileInfo[count];
			std::vector<JMacroSet> macroSet[count];
			JVector3<uint> threadDim[count];
			ID3D12RootSignature* rootSignature[count];
		private:
			uint index = 0;
		private:
			std::string errMsg;
		public:
			JDx12ComputePsoBulder(const std::string& errMsg = "")
				:errMsg(errMsg)
			{} 
		public:
			void PushHolder(JDx12ComputeShaderDataHolder* newHolder)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				holder[index] = newHolder;
			}
			void PushCompileInfo(const JCompileInfo& newCompileInfo)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				compileInfo[index] = newCompileInfo;
			}
			/**
			* push_back macro
			*/
			void PushMacroSet(const JMacroSet& newMacroSet)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				macroSet[index].push_back(newMacroSet);
			} 
			void PushMacroSetVector(const std::vector<JMacroSet>& newMacroSet)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				macroSet[index] = newMacroSet;
			}
			void PushThreadDim(const JVector3<uint>& newThreadDim)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				threadDim[index] = newThreadDim;
			}
			void PushRootSignature(ID3D12RootSignature* newRoot)
			{
				J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(index < count, "Over psoBuilder range", errMsg);
				rootSignature[index] = newRoot;
			} 
		public:
			void Next()
			{
				++index; 
				//macro set은 option이므로 미리 초기화해둔다.
				if(index < count)
					PushMacroSetVector(std::vector<JMacroSet>{});
			}
		public:
			void Create(ID3D12Device* device, D3D12_PIPELINE_STATE_FLAGS flags = D3D12_PIPELINE_STATE_FLAG_NONE)
			{ 
				const uint createCount = std::clamp(index + 1, (uint)0, count);
				for (uint i = 0; i < createCount; ++i)
				{ 
					J_LOG_PRINT_OUT_IF_FAIL_AND_RETURN(HasValidData(i), std::to_string(i) + " is invalid data", errMsg);

					JComputeShaderInitData initData;
					initData.dispatchInfo.threadDim = threadDim[i];
					initData.macro = macroSet[i];
					initData.PushThreadDimensionMacro();

					holder[i]->cs = JDxShaderDataUtil::CompileShader(compileInfo[i].filePath, initData.macro, compileInfo[i].functionName, L"cs_6_6");
					holder[i]->dispatchInfo = initData.dispatchInfo;

					JDx12ComputePso::Create(device, holder[i], rootSignature[i], D3D12_PIPELINE_STATE_FLAG_NONE);
				}
			}
		public:
			bool HasValidData(int dataIndex)const noexcept
			{
				if (dataIndex >= count)
					return false;
 
				return holder[dataIndex] != nullptr && rootSignature[dataIndex] != nullptr && threadDim[dataIndex] > JVector3F::Zero() && compileInfo[dataIndex].IsValid() ;
			}
		};

		template<uint count>
		class JDx12ComputePsoBulder<count, 0>;
		template<>
		class JDx12ComputePsoBulder<0, 0>;
	}
}