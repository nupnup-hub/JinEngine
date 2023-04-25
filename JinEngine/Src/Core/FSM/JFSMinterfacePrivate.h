#pragma once
#include"../Identity/JIdentifierPrivate.h"


namespace JinEngine
{
	namespace Core
	{
		class JFSMinterfacePrivate : public JIdentifierPrivate
		{
		public:
			class CreateInstanceInterface : public Core::JIdentifierPrivate::CreateInstanceInterface
			{
			private:
				bool Copy(JIdentifier* from, JIdentifier* to)final;
			protected:
				bool CanCopy(Core::JIdentifier* from, Core::JIdentifier* to)noexcept final;
			};   
		};
	}
}