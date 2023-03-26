#pragma once
#include<memory>
#include"../Memory/JAllocationInterface.h"
#include"../Memory/JWindowHeapAlloc.h"
#include"../Memory/JWindowVirtualAlloc.h"

namespace JinEngine
{
	namespace Core
	{  
		enum class J_ALLOCATION_TYPE
		{
			DEFAULT,
			VIRTUAL,
			HEAP,
			CUSTOM
		};

		struct JTypeAllocationOption
		{
		public:
			const J_ALLOCATION_TYPE allocationType;
			const size_t dataSize;
			const size_t allocDataCount;
		public:
			template<typename T>
			JTypeAllocationOption(const J_ALLOCATION_TYPE allocationType, const size_t allocDataCount)
				:allocationType(allocationType), dataSize(sizeof(T)), allocDataCount(allocDataCount)
			{}
			JTypeAllocationOption(const J_ALLOCATION_TYPE allocationType, const size_t dataSize, const size_t allocDataCount)
				:allocationType(allocationType), dataSize(dataSize), allocDataCount(allocDataCount)
			{}
		};

		class JTypeAllocationCreatorInterface
		{
		public:
			virtual ~JTypeAllocationCreatorInterface() = default;
		public:
			virtual std::unique_ptr<JAllocationInterface> CreateAlloc(JTypeAllocationOption* option) = 0;
		};

		template<typename T>
		class JTypeAllocationCreator : public JTypeAllocationCreatorInterface
		{
		public:
			std::unique_ptr<JAllocationInterface> CreateAlloc(JTypeAllocationOption* option) final
			{
				if (option == nullptr)
					return nullptr;

				switch (option->allocationType)
				{ 
				case JinEngine::Core::J_ALLOCATION_TYPE::VIRTUAL:
					return std::make_unique<JWindowVirtualAlloc<T>>();
				case JinEngine::Core::J_ALLOCATION_TYPE::HEAP:
					return std::make_unique<JWindowHeapAlloc<T>>();
				}
				return nullptr;
			}
		};
	}
}