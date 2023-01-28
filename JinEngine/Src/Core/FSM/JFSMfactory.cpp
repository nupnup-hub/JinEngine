#include"JFSMfactory.h"

namespace JinEngine
{
	namespace Core
	{
		bool JFSMfactory::Register(const std::string& iden, Core::JCallableInterface<JFSMInterface*, JOwnerPtr<JFSMIdentifierInitData>>* callable)
		{
			return factory.Register(iden, callable);
		}
		JFSMInterface* JFSMfactory::Create(const std::string& iden, JOwnerPtr<JFSMIdentifierInitData> initData)
		{
			return factory.Invoke(iden, std::move(initData));
		}
	}
}