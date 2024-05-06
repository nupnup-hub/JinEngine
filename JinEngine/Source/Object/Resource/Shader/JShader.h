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
#include"../JResourceObject.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h"   
#include"../../../Graphic/Shader/JShaderType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JShaderDataHolder;
	}
	class JShaderPrivate; 
	class JShader final : public JResourceObject
	{
		REGISTER_CLASS_IDENTIFIER_LINE_RESOURCE(JShader) 
	public: 
		class InitData final : public JResourceObject::InitData
		{
			REGISTER_CLASS_ONLY_USE_TYPEINFO(InitData)
		public:
			J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE;
			J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE;
			JGraphicShaderCondition condition;
		public: 
			InitData(const J_OBJECT_FLAG flag = OBJECT_FLAG_NONE,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JGraphicShaderCondition condition = JGraphicShaderCondition(),
				const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
			InitData(const std::wstring& name,
				const size_t& guid,
				const J_OBJECT_FLAG flag, 
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag = SHADER_FUNCTION_NONE,
				const JGraphicShaderCondition condition = JGraphicShaderCondition(),
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
		JUserPtr<Graphic::JShaderDataHolder> GetGraphicData(const J_GRAPHIC_RENDERING_PROCESS processType, const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<Graphic::JShaderDataHolder> GetGraphicForwardData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<Graphic::JShaderDataHolder> GetGraphicDeferredData(const J_GRAPHIC_SHADER_TYPE type, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout)const noexcept;
		JUserPtr<Graphic::JShaderDataHolder> GetComputeData()const noexcept;
		J_GRAPHIC_SHADER_FUNCTION GetShaderGFunctionFlag()const noexcept;
		J_COMPUTE_SHADER_FUNCTION GetShdaerCFunctionFlag()const noexcept;
		JGraphicShaderCondition GetShaderCondition()const noexcept;
	public:
		bool IsComputeShader()const noexcept;  
		static bool HasShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JGraphicShaderCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	public:
		static JUserPtr<JShader> FindShader(const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
			const JGraphicShaderCondition graphicPSOCond,
			const J_COMPUTE_SHADER_FUNCTION cFunctionFlag = J_COMPUTE_SHADER_FUNCTION::NONE);
	protected:
		void DoActivate()noexcept final;
		void DoDeActivate()noexcept final; 
	private:
		JShader(const InitData& initData);
		~JShader();
	};
}