#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"
#include"../../../Graphic/GraphicResource/JGraphicResourceHandleInterface.h"
#include<string>

namespace JinEngine
{ 
	class JTextureImportInterface
	{
	private:
		friend class JResourceManagerImpl;
	protected:
		virtual ~JTextureImportInterface() = default;
	public:
		virtual JTextureImportInterface* ImportInterface()noexcept = 0;
	private:
		virtual bool ImportTexture(const std::wstring& oriPath) = 0;
	};

	class JTextureInterface : public JResourceObject,
		public Graphic::JGraphicResourceHandleInterface,
		public JClearableInterface,
		public JTextureImportInterface
	{
	protected:
		JTextureInterface(const JResourceObject::JResourceInitData& initdata);
	};
}