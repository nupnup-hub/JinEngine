#include"JFileFactory.h" 
#include"JDirectory.h"
#include"../../Core/Func/Callable/JCallable.h" 
namespace JinEngine
{
	namespace
	{
		using CreateCallable = Core::JMemberCallable<JDirectory, bool, JResourceObject&>;
		using DestroyCallable = Core::JMemberCallable<JDirectory, bool, JResourceObject&>;
		static std::unique_ptr< CreateCallable> createCallable;
		static std::unique_ptr< DestroyCallable> destroyCallable;
	}

	void JFileFactoryImpl::Register(const CreateFuncPtr& cPtr, const DestroyFuncPtr& dPtr)
	{ 
		createCallable = std::make_unique<CreateCallable>(cPtr);
		destroyCallable = std::make_unique<DestroyCallable>(dPtr);
	}
	bool JFileFactoryImpl::Create(JDirectory& dir, JResourceObject& resource)
	{
		return (*createCallable)(&dir, resource);
	}
	bool JFileFactoryImpl::Destroy(JDirectory& dir, JResourceObject& resource)
	{
		return (*destroyCallable)(&dir, resource);
	}
}