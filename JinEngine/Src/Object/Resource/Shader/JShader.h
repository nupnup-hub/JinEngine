#pragma once  
#include"JShaderFunctionEnum.h" 
#include"../JResourceObject.h"
#include"../Mesh/JMeshType.h" 
#include<unordered_map> 
#include<d3d12.h>
#include<vector>
#include<memory>

namespace JinEngine
{
	struct JGraphicShaderData;
	struct JComputeShaderData;
	class JShader final : public JResourceObject
	{
		REGISTER_CLASS(JShader)
	public:
		struct JShaderInitdata : public JResourceInitData
		{
		public:
			const J_SHADER_FUNCTION shaderFunctionFlag = SHADER_FUNCTION_NONE;
			const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag = COMPUTE_SHADER_FUNCTION_NONE;
		public:
			JShaderInitdata(const size_t guid,
				const J_OBJECT_FLAG flag,
				const J_SHADER_FUNCTION shaderFunctionFlag);
			JShaderInitdata(const J_OBJECT_FLAG flag,
				const J_SHADER_FUNCTION shaderFunctionFlag);
			JShaderInitdata(const J_SHADER_FUNCTION shaderFunctionFlag);

			JShaderInitdata(const size_t guid,
				const J_OBJECT_FLAG flag,
				const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag);
			JShaderInitdata(const J_OBJECT_FLAG flag,
				const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag);
			JShaderInitdata(const J_COMPUTE_SHADER_FUNCTION cShaderFunctionFlag);
		public: 
			J_RESOURCE_TYPE GetResourceType() const noexcept;
		};
		using InitData = JShaderInitdata;
	private:
		std::unique_ptr<JGraphicShaderData>shaderData[SHADER_VERTEX_COUNT]{ nullptr, nullptr };
		std::unique_ptr< JComputeShaderData> cShaderData = nullptr;
		J_SHADER_FUNCTION functionFlag = SHADER_FUNCTION_NONE;
		J_COMPUTE_SHADER_FUNCTION cFunctionFlag = COMPUTE_SHADER_FUNCTION_NONE;
	private:
		static std::unordered_map<J_SHADER_FUNCTION, const D3D_SHADER_MACRO> shaderFuncMacroMap;
		static std::unordered_map<J_COMPUTE_SHADER_FUNCTION, const D3D_SHADER_MACRO> computeShaderFuncMacroMap;
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
		J_SHADER_FUNCTION GetShaderFunctionFlag()const noexcept;
		J_COMPUTE_SHADER_FUNCTION GetShdaerComputeFunctionFlag()const noexcept;
	public:
		bool IsComputeShader()const noexcept;
	public:
		void DoCopy(JObject* ori) final;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void SetShaderFunctionFlag(const J_SHADER_FUNCTION newFunctionFlag);
		static void CompileShdaer(JShader* shader);
		static void CompileGraphicShader(JShader* shader);
		static void CompileComputeShader(JShader* shader);
		static void GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag, const J_SHADER_FUNCTION functionFlag)noexcept;
		static void GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_COMPUTE_SHADER_FUNCTION cFunctionFlag)noexcept;
		static void GetInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept;
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