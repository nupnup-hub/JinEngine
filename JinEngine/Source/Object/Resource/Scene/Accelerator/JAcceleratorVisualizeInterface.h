#pragma once
#include"JAcceleratorType.h"
#include"../../../../Core/Guid/JGuidCreator.h"

namespace JinEngine
{ 
	class JAcceleratorVisualizeInterface
	{
	public:
		virtual ~JAcceleratorVisualizeInterface() = default;  
	public:
		virtual bool IsMatch(const J_ACCELERATOR_TYPE type)const noexcept = 0;  
	public:
		virtual void BuildNode(const std::string& name, const size_t nodeGuid, const std::string& desc = "")noexcept = 0;
		virtual void BuildEndSplit()noexcept = 0;
	};
	 
}