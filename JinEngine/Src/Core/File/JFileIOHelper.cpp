#include"JFileIOHelper.h"
#include"../../Object/JObject.h"
#include"../../Core/File/JFileConstant.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/FSM/JFSMInterface.h"
#include"../../Utility/JCommonUtility.h"

namespace JinEngine
{
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreJString(std::wofstream& stream, const std::wstring& guide, const std::wstring& str)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << " " << str << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadJString(std::wifstream& stream, std::wstring& str)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide; std::getline(stream, str);
		str = JCUtil::EraseSideWChar(str, L' ');
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat2(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT2& xm2)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << " " << xm2.x << " " << xm2.y << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat3(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT3& xm3)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << " " << xm3.x << " " << xm3.y << " " << xm3.z << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat4(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT4& xm4)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << " " << xm4.x << " " << xm4.y << " " << xm4.z << " " << xm4.w << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat4x4(std::wofstream& stream, const std::wstring& guide, const DirectX::XMFLOAT4X4& xm4x4)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << '\n';
		stream << xm4x4._11 << " " << xm4x4._12 << " " << xm4x4._13 << " " << xm4x4._14 << '\n';
		stream << xm4x4._21 << " " << xm4x4._22 << " " << xm4x4._23 << " " << xm4x4._24 << '\n';
		stream << xm4x4._31 << " " << xm4x4._32 << " " << xm4x4._33 << " " << xm4x4._34 << '\n';
		stream << xm4x4._41 << " " << xm4x4._42 << " " << xm4x4._43 << " " << xm4x4._44 << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat2(std::wifstream& stream, DirectX::XMFLOAT2& xm2)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm2.x >> xm2.y;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat3(std::wifstream& stream, DirectX::XMFLOAT3& xm3)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm3.x >> xm3.y >> xm3.z;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4(std::wifstream& stream, DirectX::XMFLOAT4& xm4)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm4.x >> xm4.y >> xm4.z >> xm4.w;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4x4(std::wifstream& stream, DirectX::XMFLOAT4X4& xm4x4)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide;
		stream >> xm4x4._11 >> xm4x4._12 >> xm4x4._13 >> xm4x4._14;
		stream >> xm4x4._21 >> xm4x4._22 >> xm4x4._23 >> xm4x4._24;
		stream >> xm4x4._31 >> xm4x4._32 >> xm4x4._33 >> xm4x4._34;
		stream >> xm4x4._41 >> xm4x4._42 >> xm4x4._43 >> xm4x4._44;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreObjectIden(std::wofstream& stream, JObject* obj)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << L"Name: " << obj->GetName() << '\n';
		stream << L"Guid: " << obj->GetGuid() << '\n';
		stream << L"Flag: " << (int)obj->GetFlag() << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadObjectIden(std::wifstream& stream, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		std::wstring name;
		int flag;

		stream >> guide; std::getline(stream, name);
		stream >> guide >> oGuid;
		stream >> guide >> flag;

		oFlag = (J_OBJECT_FLAG)flag;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadObjectIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		int flag;

		stream >> guide; std::getline(stream, oName);
		stream >> guide >> oGuid;
		stream >> guide >> flag;

		oName = JCUtil::EraseSideWChar(oName, L' ');
		oFlag = (J_OBJECT_FLAG)flag;

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreFsmObjectIden(std::wofstream& stream, Core::JFSMInterface* obj)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << L"Name: " << obj->GetName() << '\n';
		stream << L"Guid: " << obj->GetGuid() << '\n';
		stream << L"Type: " << (int)obj->GetFSMobjType() << '\n';

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadFsmObjectIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ Core::J_FSM_OBJECT_TYPE& oType)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		int fType;

		stream >> guide; std::getline(stream, oName);
		stream >> guide >> oGuid;
		stream >> guide >> fType;

		oName = JCUtil::EraseSideWChar(oName, L' ');
		oType = (Core::J_FSM_OBJECT_TYPE)fType;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasObjectIden(std::wofstream& stream, Core::JIdentifier* iden)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (iden != nullptr)
		{
			stream << L"HasObject: " << true << '\n';
			stream << L"Name: " << iden->GetName() << '\n';
			stream << Core::JFileConstant::StreamHasObjGuidSymbol() << iden->GetGuid() << '\n';
			stream << L"TypeName: " << JCUtil::StrToWstr(iden->GetTypeInfo().Name()) << '\n';
		}
		else
		{
			stream << L"HasObject: " << false << '\n';
			stream << L"Name: " << " " << L"NONE" << '\n';
			stream << Core::JFileConstant::StreamHasObjGuidSymbol() << 0 << '\n';
			stream << L"TypeName: " << " " << L"NONE" << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::JIdentifier* JFileIOHelper::LoadHasObjectIden(std::wifstream& stream)
	{
		if (!stream.is_open())
			return nullptr;

		std::wstring guide;
		std::wstring name;
		bool hasObject;
		size_t guid;
		std::wstring typeWName;
		stream >> guide >> hasObject;
		stream >> guide; std::getline(stream, name);
		stream >> guide >> guid;
		stream >> guide >> typeWName;

		if (hasObject)
			return Core::GetRawPtr(JCUtil::WstrToU8Str(typeWName), guid);
		else
			return nullptr;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::CopyFile(const std::wstring& src, const std::wstring& dest)
	{
		std::wifstream fromStream;
		fromStream.open(src, std::ios::binary | std::ios::in);
		std::wofstream toStream;
		toStream.open(dest, std::ios::binary | std::ios::out);

		if(!fromStream.is_open() || !toStream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		while (getline(fromStream, guide))
			toStream << guide << '\n';
		toStream.close();
		fromStream.close();

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	bool JFileIOHelper::SkipLine(std::wifstream& stream, const std::wstring& symbol)
	{
		std::wstring guide;
		while (getline(stream, guide))
		{
			if (JCUtil::Contain(guide, symbol))
				return true;
		}
		stream.close();
		return false;
	}
	bool JFileIOHelper::SkipSentence(std::wifstream& stream, const std::wstring& symbol)
	{
		std::wstring guide;
		while (stream >> guide)
		{
			if (JCUtil::Contain(guide, symbol))
				return true;
		}
		stream.close();
		return false;
	}
	bool JFileIOHelper::InputSpace(std::wofstream& stream, int spaceCount)
	{
		if (!stream.is_open())
			return false;

		while (spaceCount > 0)
		{
			stream << "\n";
			--spaceCount;
		}
		return true;
	}
}