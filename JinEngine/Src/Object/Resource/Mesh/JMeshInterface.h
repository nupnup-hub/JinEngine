#pragma once 
#include"../JResourceObject.h"
#include"../JClearableInterface.h" 

namespace JinEngine
{  
	struct JMeshGroup;
	class JMeshInterface : public JResourceObject,
		public JClearableInterface
	{ 
	protected:
		JMeshInterface(const JResourceObject::JResourceInitData& initdata);
	};
}