#include"JTypeAllocationCreator.h"
#include"../Memory/JAllocationDesc.h"
#include"../Memory/JAllocationInterface.h"
#include"../Memory/JHeapAlloc.h"
#include"../Memory/JVirtualAlloc.h"

namespace JinEngine
{
	namespace Core
	{
		std::unique_ptr<JAllocationInterface> JTypeAllocationCreator::CreateAlloc(JAllocationDesc* option) 
		{
			if (option == nullptr)
				return nullptr;

			switch (option->allocationType)
			{
			case JinEngine::Core::J_ALLOCATION_TYPE::VIRTUAL:
				return std::make_unique<JVirtualAlloc>();
			case JinEngine::Core::J_ALLOCATION_TYPE::HEAP:
				return std::make_unique<JHeapAlloc>();
			}
			return nullptr;
		}
	}
}