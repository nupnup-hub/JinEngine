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
		* 깊이탐색
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
		static std::string HasGuidSymbol(const bool applyUncopyableSymbol)
		{
			return applyUncopyableSymbol ? Core::JFileConstant::GetUncopiableGuidSymbol() : Core::JFileConstant::GetHasObjGuidSymbol();
		}
	}

	JFileIOTool::NextCurrentHint::NextCurrentHint(const std::string& key)
		:key(key)
	{}
	JFileIOTool::NextCurrentHint::NextCurrentHint(const int index)
		: index(index)
	{}
	JFileIOTool::NextCurrentHint* JFileIOTool::GetNextCurrentHint()const noexcept
	{
		return nextCurHint.get();
	}
	uint JFileIOTool::GetCurrentMemberCount()const noexcept
	{
		if (CanUseJSon())
			return json->GetCurrentMemberCount();
		else
			return 0;
	}
	bool JFileIOTool::CanLoad()const noexcept
	{  
		return CanUseJSon() || CanUseInputStream();
	}
	bool JFileIOTool::CanStore()const noexcept
	{
		return CanUseJSon() || CanUseOutputStream();
	}
	bool JFileIOTool::CanUseInputStream()const noexcept
	{
		return istream != nullptr && istream->is_open();
	}
	bool JFileIOTool::CanUseOutputStream()const noexcept
	{
		return ostream != nullptr && ostream->is_open();
	}
	bool JFileIOTool::CanUseJSon()const noexcept
	{
		return json != nullptr && json->HasCurrentStack();
	}
	bool JFileIOTool::Begin(const std::string& path, const TYPE toolType, const BEGIN_OPTION option)
	{
		switch (toolType)
		{
		case JinEngine::JFileIOTool::TYPE::INPUT_STREAM:
			return BeginInputStream(path);
		case JinEngine::JFileIOTool::TYPE::OUTPUT_STREAM:
			return BeginOutputStream(path);
		case JinEngine::JFileIOTool::TYPE::JSON:
		{
			if (Core::HasSQValueEnum(option, BEGIN_OPTION_JSON_TRY_LOAD_DATA))
				return BeginAndTryLoadJSon(path);
			else if (Core::HasSQValueEnum(option, BEGIN_OPTION_JSON_SET_CONTENTS_ARRAY_OWNER))
				return BeginJSon(path, true);
			else
				return BeginJSon(path, false);
		}
		default:
			break;
		}
		return false;
	}
	bool JFileIOTool::Begin(const std::wstring& path, const TYPE toolType, const BEGIN_OPTION option)
	{
		return Begin(JCUtil::WstrToU8Str(path), toolType, option);
	}
	bool JFileIOTool::BeginInputStream(const std::string& path)
	{
		if (path.empty() || istream != nullptr)
			return false;

		istream = std::make_unique<std::ifstream>();
		istream->open(path.c_str(), std::ios::in | std::ios::binary);
		if (!istream->is_open())
			istream = nullptr;
		return true;
	}
	bool JFileIOTool::BeginOutputStream(const std::string& path)
	{
		if (path.empty() || ostream != nullptr)
			return false;

		ostream = std::make_unique<std::ofstream>();
		ostream->open(path.c_str(), std::ios::out | std::ios::binary);
		if (!ostream->is_open())
			ostream = nullptr;
		return true;
	}
	bool JFileIOTool::BeginJSon(const std::string& path, const bool isContentsMemberArray)
	{
		if (path.empty() || json != nullptr)
			return false;

		json = std::make_unique<Core::JJSon>(path, isContentsMemberArray);
		return true;
	}
	bool JFileIOTool::BeginAndTryLoadJSon(const std::string& path)
	{
		if (BeginJSon(path))
			return json->Load();
		else
			return false;
	}
	void JFileIOTool::Close(const CLOSE_OPTION option)
	{
		CloseInputStream();
		CloseOutputStream();
		CloseJSon(Core::HasSQValueEnum(option, CLOSE_OPTION_JSON_STORE_DATA));
	}
	void JFileIOTool::CloseInputStream()
	{
		if (istream == nullptr)
			return;

		if (istream->is_open())
			istream->close();
		istream = nullptr;
	}
	void JFileIOTool::CloseOutputStream()
	{
		if (ostream == nullptr)
			return;

		if (ostream->is_open())
			ostream->close();
		ostream = nullptr;
	}
	void JFileIOTool::CloseJSon(const bool allowStoreJSon)
	{
		if (json == nullptr)
			return;
		if (allowStoreJSon)
			StoreJSon();
		json = nullptr;
	}
	void JFileIOTool::StoreJSon()
	{
		if (json == nullptr)
			return;

		json->Store();
	}
	bool JFileIOTool::PushArrayOwner(const std::string& key)
	{
		if (CanUseJSon())
			return json->PushArrayOwner(key);
		else if (CanUseOutputStream() && !key.empty())
		{
			(*ostream) << key << '\n';
			return true;
		}
		return false;
	}
	bool JFileIOTool::PushArrayMember()
	{
		if (CanUseJSon())
			return json->PushArrayMember();
		return CanUseOutputStream();
	}
	bool JFileIOTool::PushMapMember(const std::string& key)
	{
		if (CanUseJSon())
			return json->PushMapMember(key);
		else if (CanUseOutputStream() && !key.empty())
		{
			(*ostream) << key << '\n';
			return true;
		}
		return false;
	}
	bool JFileIOTool::PushMember(const std::string& key)
	{
		if (CanUseJSon())
		{
			if (json->IsCurrentHasArrayContainer())
				return PushArrayMember();
			else
				return PushMapMember(key);
		}
		else
		{
			if (!key.empty())
				return PushMapMember(key);
			else
				return PushArrayMember();
		}
	}
	bool JFileIOTool::PushExistStack(const std::string& key)
	{
		if (CanUseJSon())
			return json->PushExistStack(key);
		else if (CanUseInputStream() && !key.empty())
		{
			std::string guide;
			(*istream) >> guide;
			return true;
		}
		return false;
	}
	bool JFileIOTool::PopStack()
	{
		if (CanUseJSon())
			return json->PopStack();
		return true;
	}
	JFileIOTool::NextCurrentHint* JFileIOTool::CreateNextCurrentHint(std::unique_ptr<NextCurrentHint>&& hint)
	{
		nextCurHint = std::move(hint);
		return nextCurHint.get();
	}
	void JFileIOTool::ClearNextCurrentHint()
	{
		nextCurHint = nullptr;
	}

	Core::J_FILE_IO_RESULT JFileIOHelper::StoreMatrix4x4(JFileIOTool& tool, const JMatrix4x4& m, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::stringstream stream; 
			stream << m._11 << " " << m._12 << " " << m._13 << " " << m._14 << " ";
			stream << m._21 << " " << m._22 << " " << m._23 << " " << m._24 << " ";
			stream << m._31 << " " << m._32 << " " << m._33 << " " << m._34 << " ";
			stream << m._41 << " " << m._42 << " " << m._43 << " " << m._44;
			return Core::JJSon::StoreData(*tool.json, stream.str(), guide) ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			(*tool.ostream) << guide << " ";
			(*tool.ostream) << m._11 << " " << m._12 << " " << m._13 << " " << m._14 << " ";
			(*tool.ostream) << m._21 << " " << m._22 << " " << m._23 << " " << m._24 << " ";
			(*tool.ostream) << m._31 << " " << m._32 << " " << m._33 << " " << m._34 << " ";
			(*tool.ostream) << m._41 << " " << m._42 << " " << m._43 << " " << m._44 << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadMatrix4x4(JFileIOTool& tool, JMatrix4x4& m, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::string matrixStr;
			if (!Core::JJSon::LoadData(*tool.json, matrixStr, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

			std::stringstream stream(matrixStr);
			stream >> m._11 >> m._12 >> m._13 >> m._14;
			stream >> m._21 >> m._22 >> m._23 >> m._24;
			stream >> m._31 >> m._32 >> m._33 >> m._34;
			stream >> m._41 >> m._42 >> m._43 >> m._44;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide;
			(*tool.istream) >> m._11 >> m._12 >> m._13 >> m._14;
			(*tool.istream) >> m._21 >> m._22 >> m._23 >> m._24;
			(*tool.istream) >> m._31 >> m._32 >> m._33 >> m._34;
			(*tool.istream) >> m._41 >> m._42 >> m._43 >> m._44;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreJString(JFileIOTool& tool, const std::string& str, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
			return Core::JJSon::StoreData(*tool.json, str, guide) ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		else
			(*tool.ostream) << guide << ' ' << str << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreJString(JFileIOTool& tool, const std::wstring& wstr, const std::string& guide)
	{
		return StoreJString(tool, JCUtil::WstrToU8Str(wstr), guide);
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadJString(JFileIOTool& tool, std::string& str, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			if (!Core::JJSon::LoadData(*tool.json, str, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide; std::getline((*tool.istream), str);
		}
		str = JCUtil::EraseSideChar(str, ' ');
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadJString(JFileIOTool& tool, std::wstring& wstr, const std::string& key)
	{
		std::string str;
		Core::J_FILE_IO_RESULT result = LoadJString(tool, str, key);
		if (result != Core::J_FILE_IO_RESULT::SUCCESS)
			return result;

		wstr = JCUtil::U8StrToWstr(str);
		return result;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat2(JFileIOTool& tool, const DirectX::XMFLOAT2& xm2, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::stringstream stream;
			stream << xm2.x << " " << xm2.y;
			Core::JJSon::StoreData(*tool.json, stream.str(), guide);
		}
		else
			(*tool.ostream) << guide << " " << xm2.x << " " << xm2.y << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat3(JFileIOTool& tool, const DirectX::XMFLOAT3& xm3, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::stringstream stream;
			stream << xm3.x << " " << xm3.y << " " << xm3.z;
			Core::JJSon::StoreData(*tool.json, stream.str(), guide);
		}
		else
			(*tool.ostream) << guide << " " << xm3.x << " " << xm3.y << " " << xm3.z << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat4(JFileIOTool& tool, const DirectX::XMFLOAT4& xm4, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::stringstream stream;
			stream << xm4.x << " " << xm4.y << " " << xm4.z << " " << xm4.w;
			Core::JJSon::StoreData(*tool.json, stream.str(), guide);
		}
		else
			(*tool.ostream) << guide << " " << xm4.x << " " << xm4.y << " " << xm4.z << " " << xm4.w << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreXMFloat4x4(JFileIOTool& tool, const DirectX::XMFLOAT4X4& xm4x4, const std::string& guide)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::stringstream stream;
			stream << xm4x4._11 << " " << xm4x4._12 << " " << xm4x4._13 << " " << xm4x4._14 << " ";
			stream << xm4x4._21 << " " << xm4x4._22 << " " << xm4x4._23 << " " << xm4x4._24 << " ";
			stream << xm4x4._31 << " " << xm4x4._32 << " " << xm4x4._33 << " " << xm4x4._34 << " ";
			stream << xm4x4._41 << " " << xm4x4._42 << " " << xm4x4._43 << " " << xm4x4._44;
			return Core::JJSon::StoreData(*tool.json, stream.str(), guide) ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			(*tool.ostream) << guide << " ";
			(*tool.ostream) << xm4x4._11 << " " << xm4x4._12 << " " << xm4x4._13 << " " << xm4x4._14 << " ";
			(*tool.ostream) << xm4x4._21 << " " << xm4x4._22 << " " << xm4x4._23 << " " << xm4x4._24 << " ";
			(*tool.ostream) << xm4x4._31 << " " << xm4x4._32 << " " << xm4x4._33 << " " << xm4x4._34 << " ";
			(*tool.ostream) << xm4x4._41 << " " << xm4x4._42 << " " << xm4x4._43 << " " << xm4x4._44 << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat2(JFileIOTool& tool, DirectX::XMFLOAT2& xm2, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::string vecStr;
			if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

			std::stringstream stream(vecStr);
			stream >> xm2.x >> xm2.y;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide >> xm2.x >> xm2.y;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat3(JFileIOTool& tool, DirectX::XMFLOAT3& xm3, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::string vecStr;
			if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

			std::stringstream stream(vecStr);
			stream >> xm3.x >> xm3.y >> xm3.z;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide >> xm3.x >> xm3.y >> xm3.z;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4(JFileIOTool& tool, DirectX::XMFLOAT4& xm4, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::string vecStr;
			if (!Core::JJSon::LoadData(*tool.json, vecStr, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

			std::stringstream stream(vecStr);
			stream >> xm4.x >> xm4.y >> xm4.z >> xm4.w;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide >> xm4.x >> xm4.y >> xm4.z >> xm4.w;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadXMFloat4x4(JFileIOTool& tool, DirectX::XMFLOAT4X4& xm4x4, const std::string& key)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			std::string matrixStr;
			if (!Core::JJSon::LoadData(*tool.json, matrixStr, key))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;

			std::stringstream stream(matrixStr);
			stream >> xm4x4._11 >> xm4x4._12 >> xm4x4._13 >> xm4x4._14;
			stream >> xm4x4._21 >> xm4x4._22 >> xm4x4._23 >> xm4x4._24;
			stream >> xm4x4._31 >> xm4x4._32 >> xm4x4._33 >> xm4x4._34;
			stream >> xm4x4._41 >> xm4x4._42 >> xm4x4._43 >> xm4x4._44;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide;
			(*tool.istream) >> xm4x4._11 >> xm4x4._12 >> xm4x4._13 >> xm4x4._14;
			(*tool.istream) >> xm4x4._21 >> xm4x4._22 >> xm4x4._23 >> xm4x4._24;
			(*tool.istream) >> xm4x4._31 >> xm4x4._32 >> xm4x4._33 >> xm4x4._34;
			(*tool.istream) >> xm4x4._41 >> xm4x4._42 >> xm4x4._43 >> xm4x4._44;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreIden(JFileIOTool& tool, Core::JIdentifier* obj)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::StoreData(*tool.json, JCUtil::WstrToU8Str(obj->GetName()), "Name: ");
			isSuccess &= Core::JJSon::StoreData(*tool.json, obj->GetGuid(), Core::JFileConstant::GetObjGuidSymbol());
			isSuccess &= Core::JJSon::StoreData(*tool.json, obj->GetTypeInfo().TypeGuid(), Core::JFileConstant::GetTypeGuidSymbol());
			if (!isSuccess)
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			(*tool.ostream) << "Name: " << JCUtil::WstrToU8Str(obj->GetName()) << '\n';
			(*tool.ostream) << Core::JFileConstant::GetObjGuidSymbol() << obj->GetGuid() << '\n';
			(*tool.ostream) << Core::JFileConstant::GetTypeGuidSymbol() << obj->GetTypeInfo().TypeGuid() << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadIden(JFileIOTool& tool, _Out_ size_t& oGuid)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::string name;
		size_t typeGuid;
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::LoadData(*tool.json, name, "Name: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, oGuid, Core::JFileConstant::GetObjGuidSymbol());
			isSuccess &= Core::JJSon::LoadData(*tool.json, typeGuid, Core::JFileConstant::GetTypeGuidSymbol());
			if (!isSuccess)
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide; std::getline((*tool.istream), name);
			(*tool.istream) >> guide >> oGuid;
			(*tool.istream) >> guide >> typeGuid;
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		size_t typeGuid;
		std::string name;
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::LoadData(*tool.json, name, "Name: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, oGuid, Core::JFileConstant::GetObjGuidSymbol());
			isSuccess &= Core::JJSon::LoadData(*tool.json, typeGuid, Core::JFileConstant::GetTypeGuidSymbol());
			if (!isSuccess)
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide; std::getline((*tool.istream), name);
			(*tool.istream) >> guide >> oGuid;
			(*tool.istream) >> guide >> typeGuid;
		}
		oName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(name, ' '));
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreFsmIden(JFileIOTool& tool, Core::JFSMinterface* obj)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::StoreData(*tool.json, JCUtil::WstrToU8Str(obj->GetName()), "Name: ");
			isSuccess &= Core::JJSon::StoreData(*tool.json, obj->GetGuid(), Core::JFileConstant::GetUncopiableGuidSymbol());
			isSuccess &= Core::JJSon::StoreData(*tool.json, obj->GetFSMobjType(), "Type: ");
			if (!isSuccess)
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			(*tool.ostream) << "Name: " << JCUtil::WstrToU8Str(obj->GetName()) << '\n';
			(*tool.ostream) << Core::JFileConstant::GetUncopiableGuidSymbol() << obj->GetGuid() << '\n';
			(*tool.ostream) << "Type: " << (int)obj->GetFSMobjType() << '\n';
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::LoadFsmIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ Core::J_FSM_OBJECT_TYPE& oType)
	{
		if(!tool.CanLoad())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::string name;
		int fType;
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::LoadData(*tool.json, name, "Name: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, oGuid, Core::JFileConstant::GetUncopiableGuidSymbol());
			isSuccess &= Core::JJSon::LoadData(*tool.json, fType, "Type: ");
			if (!isSuccess)
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide; std::getline((*tool.istream), name);
			(*tool.istream) >> guide >> oGuid;
			(*tool.istream) >> guide >> fType;
		}
		oName = JCUtil::U8StrToWstr(JCUtil::EraseSideChar(name, ' '));
		oType = (Core::J_FSM_OBJECT_TYPE)fType;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasIden(JFileIOTool& tool, Core::JIdentifier* iden, const std::string& guide, const bool applyUncopyableSymbol)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		bool isSuccess = true;
		tool.PushMember(guide);
		if (tool.CanUseJSon())
		{
			if (iden != nullptr)
			{
				isSuccess &= Core::JJSon::StoreData(*tool.json, true, "HasObject: ");
				isSuccess &= Core::JJSon::StoreData(*tool.json, iden->GetName(), "Name: ");
				isSuccess &= Core::JJSon::StoreData(*tool.json, iden->GetGuid(), Private::HasGuidSymbol(applyUncopyableSymbol));
				isSuccess &= Core::JJSon::StoreData(*tool.json, iden->GetTypeInfo().TypeGuid(), Core::JFileConstant::GetTypeGuidSymbol());
			}
			else
			{
				isSuccess &= Core::JJSon::StoreData(*tool.json, false, "HasObject: ");
				isSuccess &= Core::JJSon::StoreData(*tool.json, "None", "Name: ");
				isSuccess &= Core::JJSon::StoreData(*tool.json, 0, Private::HasGuidSymbol(applyUncopyableSymbol));
				isSuccess &= Core::JJSon::StoreData(*tool.json, 0, Core::JFileConstant::GetTypeGuidSymbol());
			}
		}
		else
		{
			if (iden != nullptr)
			{
				(*tool.ostream) << "HasObject: " << true << '\n';
				(*tool.ostream) << "Name: " << JCUtil::WstrToU8Str(iden->GetName()) << '\n';
				(*tool.ostream) << Private::HasGuidSymbol(applyUncopyableSymbol) << iden->GetGuid() << '\n';
				(*tool.ostream) << Core::JFileConstant::GetTypeGuidSymbol() << iden->GetTypeInfo().TypeGuid() << '\n';
			}
			else
			{
				(*tool.ostream) << "HasObject: " << false << '\n';
				(*tool.ostream) << "Name: " << " " << "None" << '\n';
				(*tool.ostream) << Private::HasGuidSymbol(applyUncopyableSymbol) << 0 << '\n';
				(*tool.ostream) << Core::JFileConstant::GetTypeGuidSymbol() << " " << 0 << '\n';
			}
		}
		tool.PopStack();
		return isSuccess ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
	}
	Core::J_FILE_IO_RESULT JFileIOHelper::StoreHasInstanceHint(JFileIOTool& tool, const Core::JTypeInstanceSearchHint& hint, const std::string& guide, const bool applyUncopyableSymbol)
	{
		if(!tool.CanStore())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		bool isSuccess = true;
		tool.PushMember(guide);
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::StoreData(*tool.json, hint.isValid, "IsValid: ");
			isSuccess &= Core::JJSon::StoreData(*tool.json, hint.isValid ? hint.objectGuid : 0, Private::HasGuidSymbol(applyUncopyableSymbol));
			isSuccess &= Core::JJSon::StoreData(*tool.json, hint.isValid ? hint.typeGuid : 0, Core::JFileConstant::GetTypeGuidSymbol());
		}
		else
		{
			(*tool.ostream) << "IsValid: " << hint.isValid << '\n';
			(*tool.ostream) << Private::HasGuidSymbol(applyUncopyableSymbol) << hint.isValid ? hint.objectGuid : 0 << '\n';
			(*tool.ostream) << Core::JFileConstant::GetTypeGuidSymbol() << " " << hint.isValid ? hint.typeGuid : 0 << '\n';
		}
		tool.PopStack();
		return isSuccess ? Core::J_FILE_IO_RESULT::SUCCESS : Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
	}
	JUserPtr<Core::JIdentifier> JFileIOHelper::LoadHasIden(JFileIOTool& tool, const std::string& key, TryAgainLoadObjIfFailPtr tryAgainPtr, const bool applyUncopyableSymbol)
	{
		if(!tool.CanLoad())
			return nullptr;

		std::string name;
		bool hasObject = false;
		size_t objGuid = 0;
		size_t typeGuid = 0;

		bool isSuccess = true;
		tool.PushExistStack(key);
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::LoadData(*tool.json, hasObject, "HasObject: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, name, "Name: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, objGuid, Private::HasGuidSymbol(applyUncopyableSymbol));
			isSuccess &= Core::JJSon::LoadData(*tool.json, typeGuid, Core::JFileConstant::GetTypeGuidSymbol());
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide >> hasObject;
			(*tool.istream) >> guide; std::getline((*tool.istream), name);
			(*tool.istream) >> guide >> objGuid;
			(*tool.istream) >> guide >> typeGuid;
		}
		tool.PopStack();

		if (hasObject && isSuccess)
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
	Core::JTypeInstanceSearchHint JFileIOHelper::LoadHasIdenHint(JFileIOTool& tool, const std::string& key, TryAgainLoadHintIfFailPtr tryAgainPtr, const bool applyUncopyableSymbol)
	{
		if(!tool.CanLoad())
			return Core::JTypeInstanceSearchHint();

		bool hasObject;
		size_t objGuid;
		size_t typeGuid;

		bool isSuccess = true;
		tool.PushExistStack(key);
		if (tool.CanUseJSon())
		{
			bool isSuccess = true;
			isSuccess &= Core::JJSon::LoadData(*tool.json, hasObject, "IsValid: ");
			isSuccess &= Core::JJSon::LoadData(*tool.json, objGuid, Private::HasGuidSymbol(applyUncopyableSymbol));
			isSuccess &= Core::JJSon::LoadData(*tool.json, typeGuid, Core::JFileConstant::GetTypeGuidSymbol());
		}
		else
		{
			std::string guide;
			(*tool.istream) >> guide >> hasObject;
			(*tool.istream) >> guide >> objGuid;
			(*tool.istream) >> guide >> typeGuid;
		}
		tool.PopStack();
		if (!isSuccess)
			return Core::JTypeInstanceSearchHint();

		Core::JTypeInfo* info = _JReflectionInfo::Instance().GetTypeInfo(typeGuid);
		if (info == nullptr)
			return Core::JTypeInstanceSearchHint();

		Core::JTypeInstanceSearchHint hint(*info, objGuid);
		if (!hint.isValid && tryAgainPtr != nullptr)
			hint = tryAgainPtr(info, objGuid);

		return hint;
	}
	bool JFileIOHelper::SkipLine(std::ifstream& stream, const std::string& symbol)
	{
		std::string guide;
		while (getline(stream, guide))
		{
			if (JCUtil::Contain(guide, symbol))
				return true;
		}
		stream.close();
		return false;
	}
	bool JFileIOHelper::SkipSentence(std::ifstream& stream, const std::string& symbol)
	{
		std::string guide;
		std::string fSymbol = JCUtil::EraseSideChar(symbol, ' ');
		while (stream >> guide)
		{
			if (JCUtil::Contain(guide, fSymbol))
				return true;
		}
		stream.close();
		return false;
	}
	bool JFileIOHelper::InputSpace(JFileIOTool& tool, int spaceCount)
	{
		if (tool.CanUseOutputStream())
		{
			while (spaceCount > 0)
			{
				(*tool.ostream) << '\n';
				--spaceCount;
			}
			return true;
		}
		else
			return false;
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
			toStream << Core::JFileConstant::GetCombineFileEndW() << '\n';
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
			std::wstring symbol = Core::JFileConstant::GetCombineFileEndW();
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
		* Window에서 Directory를 지우려면 내부에 파일과 디렉토리들을 우선적으로 지워야한다.
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
	 bool JFileIOHelper::HasFile(const std::wstring& path)
	{
		 return _waccess(path.c_str(), 00) != -1;
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

	std::ifstream& JFileIOHelper::GetInputStream(JFileIOTool& tool)
	{
		return *tool.istream;
	}
	std::ofstream& JFileIOHelper::GetOutputStream(JFileIOTool& tool)
	{
		return *tool.ostream;
	}
	Core::JJSon& JFileIOHelper::GetJSonHandle(JFileIOTool& tool)
	{
		return *tool.json;
	}
}