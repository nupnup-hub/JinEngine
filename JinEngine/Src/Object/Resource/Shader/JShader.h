#pragma once  
#include"JShaderFunctionEnum.h" 
#include"JShaderData.h" 
#include"JShaderInterface.h" 
#include"../Mesh/JMeshType.h" 
#include<unordered_map> 
#include<d3d12.h>
#include<vector>
#include<memory>

namespace JinEngine
{
	class JShader final : public JShaderInterface
	{
		REGISTER_CLASS(JShader)
	private:
		struct CSInitHelper
		{
		public:
			std::vector<D3D_SHADER_MACRO> macro;
			JComputeShaderData::DispatchInfo dispatchInfo;
		};
	private:
		using CSInitHelperCallable = Core::JStaticCallable<CSInitHelper>;
	public:
		struct JShaderInitdata : public JResourceInitData
		{
		public:
			J_GRAPHIC_SHADER_FUNCTION gShaderFunctionFlag = SHADER_FUNCTION_NONE;
			J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
		public:
			JShaderInitdata(const size_t guid,
				const J_OBJECT_FLAG flag,
				const J_GRAPHIC_SHADER_FUNCTION shaderFunctionFlag,
				const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
			JShaderInitdata(const J_OBJECT_FLAG flag,
				const J_GRAPHIC_SHADER_FUNCTION shaderFunctionFlag,
				const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
			JShaderInitdata(const J_GRAPHIC_SHADER_FUNCTION shaderFunctionFlag, const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
		public:
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		private:
			std::wstring MakeName(const J_GRAPHIC_SHADER_FUNCTION shaderFunctionFlag, const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag)const noexcept;
		};
		using InitData = JShaderInitdata;
	private:
		std::unique_ptr<JGraphicShaderData>gShaderData[SHADER_VERTEX_COUNT]{ nullptr, nullptr };
		std::unique_ptr<JComputeShaderData> cShaderData = nullptr;
		J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE;
		const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
	private:
		static std::unordered_map<J_GRAPHIC_SHADER_FUNCTION, const D3D_SHADER_MACRO> shaderFuncMacroMap;  
		static std::unordered_map<J_SHADER_VERTEX_LAYOUT, const D3D_SHADER_MACRO> vertexLayoutMacroMap;
		static std::unordered_map<J_SHADER_VERTEX_LAYOUT, std::vector<D3D12_INPUT_ELEMENT_DESC>> inputLayout;
	public:
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SHADER;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		ID3D12PipelineState* GetComputePso()const noexcept;
		ID3D12PipelineState* GetGraphicPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		ID3D12RootSignature* GetComputeRootSignature()const noexcept;
		J_GRAPHIC_SHADER_FUNCTION GetShaderFunctionFlag()const noexcept;
		J_COMPUTE_SHADER_FUNCTION GetShdaerComputeFunctionFlag()const noexcept;
	public:
		bool IsComputeShader()const noexcept;
	public:
		JShaderCompileInterface* CompileInterface()noexcept;
	public:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void SetGraphicShaderFunctionFlag(const J_GRAPHIC_SHADER_FUNCTION newFunctionFlag);
		void RecompileGraphicShader()final;
		static void CompileShdaer(JShader* shader);
		static void CompileGraphicShader(JShader* shader);
		static void CompileComputeShader(JShader* shader);
		static void GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag, const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag)noexcept;
		static void GetInitHelper(_Out_ CSInitHelper& initHelper, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept;
		static void GetInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept;
	private:
		//group dim && thread dim is (n, 1, 1)
		static void CalculateThreadDim(JComputeShaderData::DispatchInfo& dInfo, const uint taskCount, const uint smCount, const uint warpFactor, const uint threadMaxDim);
		static void StuffComputeShaderThreadDim(const J_COMPUTE_SHADER_FUNCTION cFunctionFlag, const JVector3<uint> dim);
		static void OutComputeShaderCommonMacro( _Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag);
	private:
		bool HasShaderData()const noexcept;
	private:
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JShader* shader);
		static JShader* LoadObject(JDirectory* directory, const Core::JAssetFileLoadPathData& pathData);
		static void RegisterJFunc(); 
	private:
		JShader(const JShaderInitdata& initdata);
		~JShader();
	};
}