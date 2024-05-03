#include"JGraphicDevice.h" 
namespace JinEngine::Graphic
{ 
	void JGraphicDevice::StartPublicCommandSet(bool& startCommandThisFunc)
	{ 
		if (CanStartPublicCommand())
		{
			FlushCommandQueue();
			StartPublicCommand();
			startCommandThisFunc = true;
		}
		else
			startCommandThisFunc = false;
	}
	void JGraphicDevice::EndPublicCommandSet(const bool startCommandThisFunc, const bool canRestartImmdetely)
	{ 
		if (startCommandThisFunc)
		{
			EndPublicCommand();
			FlushCommandQueue();
		}
		else if(canRestartImmdetely)
			ReStartPublicCommandSet();
	}
	void JGraphicDevice::ReStartPublicCommandSet()
	{
		if (IsPublicCommandStared())
		{
			EndPublicCommand();
			FlushCommandQueue();
			StartPublicCommand();
		}
	}
}