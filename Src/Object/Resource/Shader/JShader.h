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
	struct JShaderData;
	class JShader : public JResourceObject
	{
		REGISTER_CLASS(JShader)
	private:
		std::unique_ptr<JShaderData>shaderData[SHADER_VERTEX_COUNT];
		J_SHADER_FUNCTION functionFlag;
	private:
		static std::unordered_map<J_SHADER_FUNCTION, const D3D_SHADER_MACRO> shaderFuncMacroMap;
		static std::unordered_map<J_SHADER_VERTEX_LAYOUT, const D3D_SHADER_MACRO> vertexLayoutMacroMap;
		static std::unordered_map<J_SHADER_VERTEX_LAYOUT, std::vector<D3D12_INPUT_ELEMENT_DESC>> inputLayout;
	public:	
		ID3D12PipelineState* GetPso(const J_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		J_SHADER_FUNCTION GetShaderFunctionFlag()const noexcept;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SHADER;
		}
		std::string GetFormat()const noexcept final;
		static std::vector<std::string> GetAvailableFormat()noexcept;

		static J_SHADER_FUNCTION AddShaderFunctionFlag(const J_SHADER_FUNCTION ori, const J_SHADER_FUNCTION addValue)noexcept;
		static J_SHADER_FUNCTION MinusShaderFunctionFlag(const J_SHADER_FUNCTION ori, const J_SHADER_FUNCTION minusValue)noexcept;
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final;
	private:
		void SetShaderFunctionFlag(const J_SHADER_FUNCTION newFunctionFlag);
		static void CompileShdaer(JShader* shader);
		static void GetMacroVec(_Out_ std::vector<D3D_SHADER_MACRO>& outMacro, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag, const J_SHADER_FUNCTION functionFlag)noexcept;
		static void GetInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag)noexcept;
		Core::J_FILE_IO_RESULT CallStoreResource()final;
		static Core::J_FILE_IO_RESULT StoreObject(JShader* shader);
		static JShader* LoadObject(JDirectory* directory, const JResourcePathData& pathData);
		static void RegisterFunc();
	private:
		JShader(const std::string& name, const size_t guid, const JOBJECT_FLAG objFlag, JDirectory* directory, uint8 formatIndex);
		~JShader();
	};
}