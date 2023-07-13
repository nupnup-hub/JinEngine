#pragma once
#include"../../../Core/Factory/JFactory.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"JBehavior.h"

namespace JinEngine
{ 
	class JBehavior;
	static class JBehaviorFactory : public Core::JFactory<size_t, false, false, JOwnerPtr<JBehavior>, const JinEngine::JBehavior::InitData&>
	{
	}JBehaviorFactory;
	using CreateBehaviorPtr = JBehaviorFactory::CreatePtr;
}