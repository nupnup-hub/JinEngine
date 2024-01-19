#pragma once  
#include"JShaderEnum.h" 
#include"JShaderCondition.h"  
#include"../JResourceObject.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h"   

namespace JinEngine
{
	class JShaderPrivate;
	class JGraphicShaderDataHolderBase;
	class JComputeShaderDataHolderBase;
	 
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
			JShaderCondition condition;
		public: 
			InitData(const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JShaderCondition condition = JShaderCondition(),
				const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
			InitData(const std::wstring& name,
				const size_t& guid,
				const J_OBJECT_FLAG flag, 
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JShaderCondition condition = JShaderCondition(),
				const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
		};
	private: 
		friend class JShaderPrivate;
		class JShaderImpl;
	private:
		std::unique_ptr<JShaderImpl> impl;
	public:
		Core::JIdentifierPrivate& PrivateInterface()const noexcept final;
		J_RESOURCE_TYPE GetResourceType()const noexcept final;
		static constexpr J_RESOURCE_TYPE GetStaticResourceType()noexcept
		{
			return J_RESOURCE_TYPE::SHADER;
		}
		std::wstring GetFormat()const noexcept final;
		static std::vector<std::wstring> GetAvailableFormat()noexcept;
	public:
		JUserPtr<JGraphicShaderDataHolderBase> GetGraphicData(const J_GRAPHIC_RENDERING_PROCESS processType, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<JGraphicShaderDataHolderBase> GetGraphicForwardData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<JGraphicShaderDataHolderBase> GetGraphicDeferredData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<JComputeShaderDataHolderBase> GetComputeData()const noexcept;
		JVector3<uint> GetComputeGroupDim()const noexcept;
		J_GRAPHIC_SHADER_FUNCTION GetShaderGFunctionFlag()const noexcept;
		J_COMPUTE_SHADER_FUNCTION GetShdaerCFunctionFlag()const noexcept;
		JShaderCondition GetShaderCondition()const noexcept;
	public:
		bool IsComputeShader()const noexcept;  
		static bool HasShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	public:
		static JUserPtr<JShader> FindShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JShaderCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JShader(const InitData& initData);
		~JShader();
	};
}