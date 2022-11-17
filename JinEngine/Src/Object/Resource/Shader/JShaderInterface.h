#pragma once
#include"../JResourceObject.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicImpl;
	}
	class JShaderCompileInterface
	{
	private:
		friend class Graphic::JGraphicImpl;
	protected:
		virtual ~JShaderCompileInterface() = default;
	private:
		virtual void RecompileGraphicShader() = 0;
	};

	class JShaderInterface : public JResourceObject , public JShaderCompileInterface
	{
	protected:
		JShaderInterface(const JResourceInitData& initdata);
	};
}