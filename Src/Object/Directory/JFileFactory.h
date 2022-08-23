#pragma once 

namespace JinEngine
{ 
	class JResourceObject;    
	class JDirectory;

	class JFileFactoryImpl
	{
	private:
		friend class JResourceObject;  
		friend class JDirectory;
	private:
		using CreateFuncPtr = bool(JDirectory::*)(JResourceObject&);
		using DestroyFuncPtr = bool(JDirectory::*)(JResourceObject&);
	private:
		static bool Register(const CreateFuncPtr& cPtr, const DestroyFuncPtr& dPtr);
		static bool Create(JDirectory& dir, JResourceObject& resource);
		static bool Destroy(JDirectory& dir, JResourceObject& resource);
	};
	using JFFI = JFileFactoryImpl;
}