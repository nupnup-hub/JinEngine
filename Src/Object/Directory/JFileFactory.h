#pragma once
//#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{ 
	class JResourceObject;  
	class JFileFactory
	{
	private:
		friend class JResourceObject;
		friend class JDirectory;
	private:
		using CreateFuncPtr = bool(*)(JResourceObject*);
		using EraseFuncPtr = bool(*)(JResourceObject*);
	private:
		static bool Regist(CreateFuncPtr& cPtr, EraseFuncPtr& ePtr);
		static bool Create(JResourceObject& resource);
		static bool Erase(JResourceObject& resource);
	};
}