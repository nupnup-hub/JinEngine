#pragma once 
#include"JOwnerPtr.h"

namespace JinEngine
{
	namespace Core
	{
		//¹Ì±¸Çö
		template<typename T> class JOwnerPtr;

		template<typename T>
		class JRefPtr
		{
			template<typename T> friend class JOwnerPtr;
		private:
			JOwnerPtr<T>* ownerPtr = nullptr;
			JRefPtr<T>* nextPtr = nullptr;
		public:
			JRefPtr(JRefPtr<T>* refPtr)
				:ownerPtr(refPtr->ownerPtr)
			{
				JRefPtr<T>* next = refPtr->nextPtr;
				while (next != nullptr)
					next = next->nextPtr;				
				next = this;
			}
			T* Get()noexcept
			{
				return ownerPtr.Get();
			}
		private:
			JRefPtr(T* rawPtr)
				:rawPtr(rawPtr)
			{}
		};
	}
}