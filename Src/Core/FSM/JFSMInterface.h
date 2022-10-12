#pragma once 
#include"JFSMobjectType.h"
#include"../File/JFileIOResult.h" 
#include"../Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMInterface : public JIdentifier
		{
			REGISTER_CLASS(JFSMInterface)
		protected:
			struct JFSMIdentifierInitData
			{
			public:
				std::wstring name;
				size_t guid;
			public:
				JFSMIdentifierInitData(const std::wstring& name, const size_t guid);
			};
		public:
			virtual J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept = 0;
		public:
			void Destroy();
		protected:
			virtual void Clear() = 0;
		protected:
			JFSMInterface(const std::wstring& name, const size_t guid);
		};
	}
}