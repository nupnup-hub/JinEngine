#pragma once
#include<string>
#include"JFSMObjectType.h"
#include"../File/JFileIOResult.h"

namespace JinEngine
{
	namespace Core
	{
		class JFSMIdentifier
		{
		protected:
			struct JFSMIdentifierData
			{
			public:
				std::wstring name;
				size_t guid;
				int fsmObjType;
			};
		private:
			std::wstring name;
			const size_t guid;
		protected:
			JFSMIdentifier(const std::wstring& name, const size_t guid);
			virtual ~JFSMIdentifier();
		public:
			std::wstring GetName()const noexcept; 
			size_t GetGuid()const noexcept;
		public:
			virtual J_FSM_OBJECT_TYPE GetFSMobjType()const noexcept = 0;
		public:
			virtual void SetName(const std::wstring& newName)noexcept; 
		protected:
			static J_FILE_IO_RESULT StoreIdentifierData(std::wofstream& stream, JFSMIdentifier& iden);
			static J_FILE_IO_RESULT LoadIdentifierData(std::wifstream& stream, JFSMIdentifierData& data);
		};
	}
}