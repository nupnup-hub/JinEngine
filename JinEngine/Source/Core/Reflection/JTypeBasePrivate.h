#pragma once
#include"../Pointer/JOwnerPtr.h"

namespace JinEngine
{ 
	class JDirectory;
	class JDirectoryPrivate;
	namespace Core
	{
		class JTypeBase;
		class JIdentifier;
		class JIdentifierPrivate;
		class JTypeBasePrivate
		{
		public:
			class InstanceInterface
			{
			private:
				friend class JIdentifier;
				friend class JIdentifierPrivate;
				friend class JDirectory;		// create file
				friend class JDirectoryPrivate;		// create file  
			private:
				static bool AddInstance(JOwnerPtr<JTypeBase>&& ownerPtr)noexcept;
				static bool RemoveInstance(JTypeBase* ptr)noexcept; 
			};
		};
	}
}