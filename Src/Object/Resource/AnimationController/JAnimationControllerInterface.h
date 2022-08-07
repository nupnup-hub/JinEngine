#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 

namespace JinEngine
{
	class JAnimationControllerInterface : public JResourceObject, public JClearableInterface
	{
	protected:
		JAnimationControllerInterface(const std::string& name, const size_t guid, const JOBJECT_FLAG flag, JDirectory* directory, const uint8 formatIndex);
	};
}