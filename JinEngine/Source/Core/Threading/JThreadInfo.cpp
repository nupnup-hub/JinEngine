#include"JThreadInfo.h"   
#include"../Guid/JGuidCreator.h"

namespace JinEngine
{
	namespace Core
	{
		JThreadInitInfo::JThreadInitInfo(ThreadEndNotifyPtr* notifyF)
			:notifyF(notifyF)
		{}
		JThreadInfo::JThreadInfo(const JThreadInitInfo& initInfo, const J_THREAD_USE_CASE_TYPE useCase)
			: guid(MakeGuid()),
			useCase(useCase),
			notifyF(initInfo.notifyF),
			state(J_THREAD_STATE_TYPE::WAIT),
			callBindOnce(initInfo.callBindOnce)
		{}
	}
}