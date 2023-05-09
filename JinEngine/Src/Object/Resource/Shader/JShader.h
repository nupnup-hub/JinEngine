#pragma once  
#include"JShaderFunctionEnum.h" 
#include"JShaderGraphicPsoCondition.h"
#include"JShaderData.h"  
#include"JExtraPsoType.h"
#include"../JResourceObject.h"
#include"../Mesh/JMeshType.h"  
#include<d3d12.h>  

namespace JinEngine
{
	class JShaderPrivate;
	class JShader final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE(JShader)
		REGISTER_CLASS_IDENTIFIER_DEFAULT_LAZY_DESTRUCTION
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE;
			J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
			JShaderGraphicPsoCondition graphicPSOCond;
		public: 
			InitData(const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JShaderGraphicPsoCondition graphicPSOCond = JShaderGraphicPsoCondition(),
				const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
			InitData(const std::wstring& name,
				const size_t& guid,
				const J_OBJECT_FLAG flag, 
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JShaderGraphicPsoCondition graphicPSOCond = JShaderGraphicPsoCondition(),
				const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
		};
	private: 
		friend class JShaderPrivate;
		class JShaderImpl;
	private:
		std::unique_ptr<JShaderImpl> impl;
	public:
		Core::JIdentifierPrivate& GetPrivateInterface()const noexcept final;
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
		ID3D12PipelineState* GetGraphicExtraPso(const J_SHADER_VERTEX_LAYOUT vertexLayout, const J_GRAPHIC_EXTRA_PSO_TYPE type)const noexcept;
		ID3D12RootSignature* GetComputeRootSignature()const noexcept;
		JVector3<uint> GetComputeGroupDim()const noexcept;
		J_GRAPHIC_SHADER_FUNCTION GetShaderGFunctionFlag()const noexcept;
		J_COMPUTE_SHADER_FUNCTION GetShdaerCFunctionFlag()const noexcept;
		JShaderGraphicPsoCondition GetSubGraphicPso()const noexcept;
	public:
		bool IsComputeShader()const noexcept;  
		static bool HasShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderGraphicPsoCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	public:
		static JUserPtr<JShader> FindShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderGraphicPsoCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JShader(const InitData& initData);
		~JShader();
	};
}