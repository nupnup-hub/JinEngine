#pragma once
#include"../Identity/JIdentifierPrivate.h"


namespace JinEngine
{
	namespace Core
	{
		class JFSMinterfacePrivate : public JIdentifierPrivate
		{
		public:
			class CreateInstanceInterface : public JIdentifierPrivate::CreateInstanceInterface
			{
			private:
				bool Copy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)final;
			protected:
				bool CanCopy(JUserPtr<JIdentifier> from, JUserPtr<JIdentifier> to)noexcept final;
			};   
		};
	}
}