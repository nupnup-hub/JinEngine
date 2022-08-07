#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"
#include"../../../Graphic/JGraphicTexture.h"
#include<string>

namespace JinEngine
{ 
	class JTextureInterface : public JResourceObject, public Graphic::JGraphicTexture , public JClearableInterface
	{
	protected:
		JTextureInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const int formatIndex);
	};
}