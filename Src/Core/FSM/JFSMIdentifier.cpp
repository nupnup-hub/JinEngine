#include"JFSMIdentifier.h"
#include"../../Utility/JCommonUtility.h"
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JFSMIdentifier::JFSMIdentifier(const std::wstring& name, const size_t guid)
			:name(name), guid(guid)
		{}
		JFSMIdentifier::~JFSMIdentifier(){}
		std::wstring JFSMIdentifier::GetName()const noexcept
		{
			return name;
		}
		size_t JFSMIdentifier::GetGuid()const noexcept
		{
			return guid;
		}
		void JFSMIdentifier::SetName(const std::wstring& name)noexcept
		{
			if (!name.empty())
				JFSMIdentifier::name = name;
		}
		J_FILE_IO_RESULT JFSMIdentifier::StoreIdentifierData(std::wofstream& stream, JFSMIdentifier& iden)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

			stream << iden.GetName() << '\n';
			stream << iden.GetGuid() << '\n';
			stream << (int)iden.GetFSMobjType() << '\n';

			return J_FILE_IO_RESULT::SUCCESS;
		}
		J_FILE_IO_RESULT JFSMIdentifier::LoadIdentifierData(std::wifstream& stream, JFSMIdentifierData& data)
		{
			if (!stream.is_open())
				return J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
 
			stream >> data.name;
			stream >> data.guid;
			stream >> data.fsmObjType;

			return J_FILE_IO_RESULT::SUCCESS;
		}
	};
}
