#pragma once
#include"../Memory/JAllocationInterface.h"
#include<memory>

namespace JinEngine
{
	namespace Core
	{ 
		class JTypeAllocationCreatorInterface
		{
		public:
			virtual std::unique_ptr<JAllocationInterface> CreateAlloc(JAllocationDesc* option) = 0;
		};

		class JTypeAllocationCreator : public JTypeAllocationCreatorInterface
		{
		public:
			std::unique_ptr<JAllocationInterface> CreateAlloc(JAllocationDesc* option) final;
		};
	}
}