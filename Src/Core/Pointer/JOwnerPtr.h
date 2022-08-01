#pragma once
#include"JRefPtr.h"

namespace JinEngine
{
	namespace Core
	{	
		//미구현
		template<typename T> class JRefPtr;

		template<typename T>
		class JOwnerPtr
		{ 
		private:
			T* rawPtr = nullptr;
			JRefPtr<T>* nextPtr = nullptr;
		public:
			JOwnerPtr(T* rawPtr)
				:rawPtr(rawPtr)
			{}
			T* Get()noexcept
			{
				return rawPtr;
			}
			JRefPtr<T>* AddRef()noexcept
			{
				if (nextPtr != nullptr)
					return new JRefPtr<T>(nextPtr);
				else
				{
					nextPtr = new JRefPtr<T>(rawPtr);
					return nextPtr;
				}
			}
			void Reset()
			{
				//수정필요
			}
		};
	}
}