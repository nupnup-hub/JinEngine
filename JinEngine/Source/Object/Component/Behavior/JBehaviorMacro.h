#pragma once
#include"JBehaviorFactory.h"
#include"JBehavior.h"

#define REGISTER_BEHAVIOR_FACTORY(name)			\
private:										\
	inline static struct name##BehaviorRegister	\
	{											\
	public:										\
		name##BehaviorRegister()				\
		{										\
			auto creaetPtr = [](const JinEngine::JBehavior::InitData& initData)					\
			{																					\
				return JinEngine::Core::JPtrUtil::MakeOwnerPtr<JinEngine::JBehavior>(initData);	\
			};																					\
			JinEngine::JBehaviorFactory.Register(#name, creaetPtr);								\
		}																						\
	}name##BehaviorRegister;																	\
};																								\