#include"JFileIOHelper.h"  
#include"../../Core/File/JFileConstant.h"
#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/FSM/JFSMinterface.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Identity/JIdentifier.h"

namespace JinEngine
{
	namespace Private
	{
		using TraversalPtr = void(*)(const std::wstring& path, const bool isDir);
		struct TraversalCondition
		{
		public:
			bool canAccessDir = true;
			bool canAccessFile = true;
			bool callDirPtrAfterTrabersal = true;
		};

		/**
		* ����Ž��
		*/
		static void TraversalDirectroy(const std::wstring& parentDirectoryPath, 
			TraversalPtr ptr,
			const TraversalCondition& cond)
		{
			if (ptr == nullptr)
				return;

			WIN32_FIND_DATA  findFileData;
			HANDLE hFindFile = FindFirstFile((parentDirectoryPath + L"\\*.*").c_str(), &findFileData);
			BOOL bResult = TRUE;
			if (hFindFile == INVALID_HANDLE_VALUE)
				return;

			if (cond.canAccessDir && !cond.callDirPtrAfterTrabersal)
				ptr(parentDirectoryPath + L"\\" + findFileData.cFileName, true);
			while (bResult)
			{
				const bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
				if (!isDir)
				{
					if (cond.canAccessFile && wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
						ptr(parentDirectoryPath + L"\\" + findFileData.cFileName, isDir);
				} 
				if (isDir)
				{
					if (wcscmp(findFileData.cFileName, L".") && wcscmp(findFileData.cFileName, L".."))
						TraversalDirectroy(parentDirectoryPath + L"\\" + findFileData.cFileName, ptr, cond); 
				}
				bResult = FindNextFile(hFindFile, &findFileData);
			}
			FindClose(hFindFile);
			if (cond.canAccessDir && cond.callDirPtrAfterTrabersal)
				ptr(parentDirectoryPath, true);
		}
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreMatrix4x4(std::wofstream& stream, const std::wstring& guide, const JMatrix4x4& m)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << guide << '\n';
		stream << m._11 << " " << m._12 << " " << m._13 << " " << m._14 << '\n';
		stream << m._21 << " " << m._22 << " " << m._23 << " " << m._24 << '\n';
		stream << m._31 << " " << m._32 << " " << m._33 << " " << m._34 << '\n';
		stream << m._41 << " " << m._42 << " " << m._43 << " " << m._44 << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadMatrix4x4(std::wifstream& stream, JMatrix4x4& m)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide;
		stream >> m._11 >> m._12 >> m._13 >> m._14;
		stream >> m._21 >> m._22 >> m._23 >> m._24;
		stream >> m._31 >> m._32 >> m._33 >> m._34;
		stream >> m._41 >> m._42 >> m._43 >> m._44;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreJString(std::wofstream& stream, const std::wstring& guide, const std::wstring& str, const bool useChangeLine)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if(useChangeLine)
			stream << guide << L' ' << str << L'\n';
		else
			stream << guide << L' ' << str << L'\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadJString(std::wifstream& stream, std::wstring& str)
	{
		if (!stream.is_open() || stream.eof())
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
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm2.x >> xm2.y;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat3(std::wifstream& stream, DirectX::XMFLOAT3& xm3)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm3.x >> xm3.y >> xm3.z;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4(std::wifstream& stream, DirectX::XMFLOAT4& xm4)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide >> xm4.x >> xm4.y >> xm4.z >> xm4.w;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4x4(std::wifstream& stream, DirectX::XMFLOAT4X4& xm4x4)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		stream >> guide;
		stream >> xm4x4._11 >> xm4x4._12 >> xm4x4._13 >> xm4x4._14;
		stream >> xm4x4._21 >> xm4x4._22 >> xm4x4._23 >> xm4x4._24;
		stream >> xm4x4._31 >> xm4x4._32 >> xm4x4._33 >> xm4x4._34;
		stream >> xm4x4._41 >> xm4x4._42 >> xm4x4._43 >> xm4x4._44;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	static Core::J_FILE_IO_RESULT StoreIden(std::wofstream& stream, Core::JIdentifier* obj);
	static Core::J_FILE_IO_RESULT LoadIden(std::wifstream& stream, _Out_ size_t& oGuid);
	static Core::J_FILE_IO_RESULT LoadIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid);
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreIden(std::wofstream& stream, Core::JIdentifier* obj)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << L"Name: " << obj->GetName() << '\n';
		stream << Core::JFileConstant::StreamObjGuidSymbol() << obj->GetGuid() << '\n';
		stream << Core::JFileConstant::StreamTypeGuidSymbol() << obj->GetTypeInfo().TypeGuid() << '\n'; 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadIden(std::wifstream& stream, _Out_ size_t& oGuid)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		std::wstring name;  
		size_t typeGuid;

		stream >> guide; std::getline(stream, name); 
		stream >> guide >> oGuid;
		stream >> guide >> typeGuid; 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide; 
		size_t typeGuid;

		stream >> guide; std::getline(stream, oName);
		stream >> guide >> oGuid;
		stream >> guide >> typeGuid; 

		oName = JCUtil::EraseSideWChar(oName, L' '); 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreFsmIden(std::wofstream& stream, Core::JFSMinterface* obj)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << L"Name: " << obj->GetName() << '\n';
		stream << Core::JFileConstant::StreamUncopiableGuidSymbol() << obj->GetGuid() << '\n';
		stream << L"Type: " << (int)obj->GetFSMobjType() << '\n';

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadFsmIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ Core::J_FSM_OBJECT_TYPE& oType)
	{
		if (!stream.is_open() || stream.eof())
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
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden)
	{
		return StoreHasIden(stream, iden, Core::JFileConstant::StreamHasObjGuidSymbol());
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden, const std::wstring& guiSymbol)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (iden != nullptr)
		{
			stream << L"HasObject: " << true << '\n';
			stream << L"Name: " << iden->GetName() << '\n';
			stream << guiSymbol << iden->GetGuid() << '\n';
			stream << Core::JFileConstant::StreamTypeGuidSymbol()<< iden->GetTypeInfo().TypeGuid() << '\n';
		}
		else
		{
			stream << L"HasObject: " << false << '\n';
			stream << L"Name: " << " " << L"NONE" << '\n';
			stream << guiSymbol << 0 << '\n';
			stream << Core::JFileConstant::StreamTypeGuidSymbol() << " " << 0 << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasInstanceHint(std::wofstream& stream, const Core::JTypeInstanceSearchHint& hint)
	{
		return StoreHasInstanceHint(stream, hint, Core::JFileConstant::StreamHasObjGuidSymbol());
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasInstanceHint(std::wofstream& stream, const Core::JTypeInstanceSearchHint& hint, const std::wstring& guiSymbol)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		stream << L"IsValid: " << hint.isValid << '\n'; 
		stream << guiSymbol << hint.objectGuid << '\n';
		stream << Core::JFileConstant::StreamTypeGuidSymbol() << " " << hint.typeGuid << '\n';

		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	JUserPtr<Core::JIdentifier> JFileIOHelper::LoadHasIden(std::wifstream& stream, TryAgainLoadObjIfFailPtr tryAgainPtr)
	{
		if (!stream.is_open() || stream.eof())
			return JUserPtr<Core::JIdentifier>{};

		std::wstring guide;
		std::wstring name;
		bool hasObject;
		size_t objGuid;
		size_t typeGuid;
		stream >> guide >> hasObject;
		stream >> guide; std::getline(stream, name);
		stream >> guide >> objGuid;
		stream >> guide >> typeGuid;

		if (hasObject)
		{
			auto rawPtr = Core::GetRawPtr(typeGuid, objGuid);
			if (rawPtr == nullptr)
			{
				Core::JTypeInfo* typeInfo = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
				if (typeInfo != nullptr && tryAgainPtr != nullptr)
					return tryAgainPtr(typeInfo, objGuid);
				else
					return JUserPtr<Core::JIdentifier>{};	 
			}
			else
				return Core::GetUserPtr<Core::JIdentifier>(rawPtr);
		}
		else
			return JUserPtr<Core::JIdentifier>{};
	}
	Core::JTypeInstanceSearchHint JFileIOHelper::LoadHasIdenHint(std::wifstream& stream, TryAgainLoadHintIfFailPtr tryAgainPtr)
	{
		if (!stream.is_open() || stream.eof())
			return Core::JTypeInstanceSearchHint{};

		std::wstring guide; 
		bool hasObject;
		size_t objGuid;
		size_t typeGuid;
		stream >> guide >> hasObject; 
		stream >> guide >> objGuid;
		stream >> guide >> typeGuid;

		Core::JTypeInfo* info = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
		if (info == nullptr)
			return Core::JTypeInstanceSearchHint();

		Core::JTypeInstanceSearchHint hint(*info, objGuid);
		if (!hint.isValid && tryAgainPtr != nullptr)
			hint = tryAgainPtr(info, objGuid);

		return hint;
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
		std::wstring fSymbol = JCUtil::EraseSideWChar(symbol, L' ');
		while (stream >> guide)
		{
			if (JCUtil::Contain(guide, fSymbol))
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
	Core::J_FILE_IO_RESULT JFileIOHelper::CopyFile(const std::wstring& from, const std::wstring& to)
	{
		std::wifstream fromStream;
		std::wofstream toStream;

		fromStream.open(from, std::ios::binary | std::ios::in);
		toStream.open(to, std::ios::binary | std::ios::out);

		if (!fromStream.is_open() || !toStream.is_open())
		{
			fromStream.close();
			toStream.close();
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
		}

		toStream << fromStream.rdbuf();
		fromStream.close();
		toStream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::CombineFile(const std::vector<std::wstring> from, const std::wstring& to)
	{
		std::wifstream fromStream;
		std::wofstream toStream;

		toStream.open(to, std::ios::binary | std::ios::out);
		const uint fromCount = (uint)from.size();
		for (uint i = 0; i < fromCount; ++i)
		{
			fromStream.open(from[i], std::ios::binary | std::ios::in);
			if (!fromStream.is_open() || !toStream.is_open())
			{
				fromStream.close();
				toStream.close();
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			}

			std::wstring guide;
			while (getline(fromStream, guide))
				toStream << guide << '\n';
			toStream << Core::JFileConstant::StreamCombineFileEnd() << '\n';
			fromStream.close();
		}
		toStream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::DevideFile(const std::wstring& from, const std::vector<std::wstring> to)
	{
		std::wifstream fromStream;
		std::wofstream toStream;

		fromStream.open(from, std::ios::binary | std::ios::in);
		const uint toCount = (uint)to.size();
		for (uint i = 0; i < toCount; ++i)
		{
			toStream.open(to[i], std::ios::binary | std::ios::out);
			if (!fromStream.is_open() || !toStream.is_open())
			{
				fromStream.close();
				toStream.close();
				return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;
			}

			std::wstring guide;
			std::wstring symbol = Core::JFileConstant::StreamCombineFileEnd();
			while (getline(fromStream, guide))
			{
				if (JCUtil::Contain(guide, symbol))
					break;
				toStream << guide << '\n';
			}		 
			toStream.close();
		}
		fromStream.close();
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::DestroyFile(const std::wstring& path)
	{
		if (_waccess(path.c_str(), 00) == -1)
			return 	Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
 
		return _wremove(path.c_str()) == 0 ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::DestroyDirectory(const std::wstring& path)
	{
		if (_waccess(path.c_str(), 00) == -1)
			return 	Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
	  
		/**
		* Window���� Directory�� ������� ���ο� ���ϰ� ���丮���� �켱������ �������Ѵ�.
		*/
		Private::TraversalPtr destroyPtr = [](const std::wstring& path, const bool isDir)
		{
			if (isDir)
				_wrmdir(path.c_str()); 
			else
				_wremove(path.c_str());
		};
		Private::TraversalCondition cond;
		cond.canAccessDir = cond.canAccessFile = cond.callDirPtrAfterTrabersal = true;

		Private::TraversalDirectroy(path.c_str(), destroyPtr, cond);
		return _wrmdir(path.c_str()) == 0 ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
	}
	std::string JFileIOHelper::FileToString(const std::string& path)
	{
		std::ifstream stream; 
		stream.open(path, std::ios::binary | std::ios::in);
		std::string content((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
		stream.close();

		return content;
	}
	std::wstring JFileIOHelper::FileToWString(const std::wstring& path)
	{
		std::wifstream stream;
		stream.open(path, std::ios::binary | std::ios::in);
		std::wstring content((std::istreambuf_iterator<wchar_t>(stream)), std::istreambuf_iterator<wchar_t>());
		stream.close();

		return content;
	}
}