#pragma once
#include"../JResourceObject.h"
#include"../JClearableInterface.h"
#include"../JResourceUserInterface.h"

namespace JinEngine
{
	namespace Core
	{
		struct JFbxAnimationData;
	}

	class JAnimationClipInterface : public JResourceObject, 
		public JClearableInterface, 
		public JResourceUserInterface
	{
	protected:
		JAnimationClipInterface(const JResourceObject::JResourceInitData& initdata);
	};
}