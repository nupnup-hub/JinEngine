#pragma once 
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 
#include"../JResourceUserInterface.h"

namespace JinEngine
{ 
	class JModelInterface : public JResourceObject, public JClearableInterface, public JResourceUserInterface
	{
	protected:
		JModelInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}