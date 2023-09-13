#pragma once 
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_PARAMETER_TYPE : uint8
		{
			UnKnown,
			Void,
			Bool, 
			Int, //short , int, long,  size_t 
			Float, 
			String,
			JVector2,	//Reflection support JVector<float> or JVector<int> or JVector<std::string>
			JVector3,
			JVector4,
			XMInt2,
			XMInt3,
			XMInt4,
			XMFloat2,
			XMFloat3,
			XMFloat4,
			STD_VECTOR,	//Reflection support std::vector<T> T = AtomicType..
						// std::vector<Class<T>> is not support gui
			STD_DEQUE,
			STD_MAP,
			STD_UNORDERED_MAP,
			USER_PTR,
			Class,
			Enum,
		};
	}
}