#include"JFileFactory.h" 
#include"../../Core/Func/Callable/JCallable.h" 

namespace JinEngine
{
	class JFileFactory
	{
	public:
		using CreateCallable = Core::JMemeberCallable<JDirectory, bool, JResourceObject&>;
		using DestroyCallable = Core::JMemeberCallable<JDirectory, bool, JResourceObject&>;
	public:
		static CreateCallable* createCallable;  
		static DestroyCallable* destroyCallable; 
	};

	JFileFactory::CreateCallable* JFileFactory::createCallable;  
	JFileFactory::DestroyCallable* JFileFactory::destroyCallable; 

	bool JFileFactoryImpl::Register(const CreateFuncPtr& cPtr, const DestroyFuncPtr& dPtr)
	{
		using CreateCallable = JFileFactory::CreateCallable; 
		static CreateCallable crateCallable{ cPtr }; 
		JFileFactory::createCallable = &crateCallable;

		using DestroyCallable = JFileFactory::DestroyCallable;
		static DestroyCallable destroyCallable{ dPtr };
		JFileFactory::destroyCallable = &destroyCallable;
	}
	bool JFileFactoryImpl::Create(JDirectory& dir, JResourceObject& resource)
	{ 
		return (*JFileFactory::createCallable)(&dir, resource);
	}
	bool JFileFactoryImpl::Destroy(JDirectory& dir, JResourceObject& resource)
	{
		return (*JFileFactory::destroyCallable)(&dir, resource);
	}
}