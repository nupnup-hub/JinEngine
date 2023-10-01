#include"JObjectFileIOHelper.h"
#include"Component/JComponent.h"
#include"Resource/JResourceObject.h"
#include"Resource/JResourceManager.h"
#include"../Core/File/JFileConstant.h"

namespace JinEngine
{
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::StoreObjectIden(std::wofstream& stream, JObject* obj)
	{
		if (!stream.is_open())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		Core::J_FILE_IO_RESULT res = StoreIden(stream, obj);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;
		 
		stream << Core::JFileConstant::StreamObjFlagSymbol() << (int)obj->GetFlag() << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadObjectIden(std::wifstream& stream, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		int flag;

		Core::J_FILE_IO_RESULT res = LoadIden(stream, oGuid);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream >> guide >> flag;
		oFlag = (J_OBJECT_FLAG)flag;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadObjectIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{
		if (!stream.is_open() || stream.eof())
			return Core::J_FILE_IO_RESULT::FAIL_STREAM_ERROR;

		std::wstring guide;
		int flag;
		size_t typeGuid;

		Core::J_FILE_IO_RESULT res = LoadIden(stream, oName, oGuid);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream >> guide >> flag;	 
		oFlag = (J_OBJECT_FLAG)flag;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::StoreComponentIden(std::wofstream& stream, JComponent* comp)
	{
		Core::J_FILE_IO_RESULT res = StoreObjectIden(stream, comp);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		stream << Core::JFileConstant::StreamActivatedSymbol() << comp->IsActivated() << "\n";
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadComponentIden(std::wifstream& stream, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated)
	{
		Core::J_FILE_IO_RESULT res = LoadObjectIden(stream, oGuid, oFlag);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		std::wstring guide;
		stream >> guide >> isActivated;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadComponentIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated)
	{
		Core::J_FILE_IO_RESULT res = LoadObjectIden(stream, oName, oGuid, oFlag);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		std::wstring guide;
		stream >> guide >> isActivated;
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::_StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden)
	{
		return JFileIOHelper::StoreHasIden(stream, iden);
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::_StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden, const std::wstring& guiSymbol)
	{
		return JFileIOHelper::StoreHasIden(stream, iden, guiSymbol);
	}
	JUserPtr<Core::JIdentifier> JObjectFileIOHelper::_LoadHasIden(std::wifstream& stream)
	{
		TryAgainLoadObjIfFailPtr ptr = [](Core::JTypeInfo* info, size_t objGuid) 
			->  JUserPtr<Core::JIdentifier>
		{
			if (info->IsChildOf<JResourceObject>())
				return _JResourceManager::Instance().TryGetResourceUser(*info, objGuid);
			else
				return nullptr;
		}; 

		return LoadHasIden(stream, ptr);
	}
}