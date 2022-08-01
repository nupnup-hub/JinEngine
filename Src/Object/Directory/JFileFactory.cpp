#include"JFileFactory.h" 
#include"../../Core/Func/Callable/JCallable.h"

namespace JinEngine
{
	class JFileFactoryFunc
	{
	public:
		using CreateCallable = Core::JStaticCallable<bool, JResourceObject*>;
		using EraseCallable = Core::JStaticCallable<bool, JResourceObject*>;
	public:
		static CreateCallable* createCallable;
		static EraseCallable* eraseCallable;
	};

	JFileFactoryFunc::CreateCallable* JFileFactoryFunc::createCallable;
	JFileFactoryFunc::EraseCallable* JFileFactoryFunc::eraseCallable;

	bool JFileFactory::Regist(CreateFuncPtr& cPtr, EraseFuncPtr& ePtr)
	{
		using CreateCallable = JFileFactoryFunc::CreateCallable;
		using EraseCallable = JFileFactoryFunc::EraseCallable;

		static CreateCallable crateCallable{ cPtr };
		static EraseCallable eraseCallable{ ePtr };
		 
		JFileFactoryFunc::createCallable = &crateCallable;
		JFileFactoryFunc::eraseCallable = &eraseCallable;
	}
	bool JFileFactory::Create(JResourceObject& resource)
	{
		return (*JFileFactoryFunc::createCallable)(nullptr, &resource);
	}
	bool JFileFactory::Erase(JResourceObject& resource)
	{
		return (*JFileFactoryFunc::eraseCallable)(nullptr, &resource);
	}
}