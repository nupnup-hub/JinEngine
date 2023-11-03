#include"JObjectFileIOHelper.h"
#include"Component/JComponent.h"
#include"Resource/JResourceObject.h"
#include"Resource/JResourceManager.h"
#include"../Core/File/JFileConstant.h"

namespace JinEngine
{
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::StoreObjectIden(JFileIOTool& tool, JObject* obj)
	{ 
		Core::J_FILE_IO_RESULT res = StoreIden(tool, obj);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;
		 
		if (tool.CanUseJSon())
		{
			if(!Core::JJSon::StoreData(GetJSonHandle(tool), obj->GetFlag(), Core::JFileConstant::GetObjFlagSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
			GetOutputStream(tool) << Core::JFileConstant::GetObjFlagSymbol() << (int)obj->GetFlag() << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	} 
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadObjectIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{
		Core::J_FILE_IO_RESULT res = LoadIden(tool, oGuid);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (tool.CanUseJSon())
		{
			J_OBJECT_FLAG flag;
			if(!Core::JJSon::LoadData(GetJSonHandle(tool), flag, Core::JFileConstant::GetObjFlagSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			oFlag = flag;
		}
		else
		{
			std::string guide;
			int flag;
			GetInputStream(tool) >> guide >> flag;
			oFlag = (J_OBJECT_FLAG)flag; 
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadObjectIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag)
	{  
		Core::J_FILE_IO_RESULT res = LoadIden(tool, oName, oGuid);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (tool.CanUseJSon())
		{
			J_OBJECT_FLAG flag;
			if (!Core::JJSon::LoadData(GetJSonHandle(tool), flag, Core::JFileConstant::GetObjFlagSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			oFlag = flag;
		}
		else
		{
			std::string guide;
			int flag;
			GetInputStream(tool) >> guide >> flag;
			oFlag = (J_OBJECT_FLAG)flag;
		} 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::StoreComponentIden(JFileIOTool& tool, JComponent* comp)
	{
		Core::J_FILE_IO_RESULT res = StoreObjectIden(tool, comp);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (tool.CanUseJSon())
		{
			if (!Core::JJSon::StoreData(GetJSonHandle(tool), comp->IsActivated(), Core::JFileConstant::GetActivatedSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
		}
		else
			GetOutputStream(tool) << Core::JFileConstant::GetActivatedSymbol() << comp->IsActivated() << '\n';
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadComponentIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated)
	{
		Core::J_FILE_IO_RESULT res = LoadObjectIden(tool, oGuid, oFlag);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (tool.CanUseJSon())
		{
			bool temp;
			if (!Core::JJSon::LoadData(GetJSonHandle(tool), temp, Core::JFileConstant::GetActivatedSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			isActivated = temp;
		}
		else
		{
			std::string guide; 
			GetInputStream(tool) >> guide >> isActivated; 
		}
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::LoadComponentIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated)
	{ 
		Core::J_FILE_IO_RESULT res = LoadObjectIden(tool, oName, oGuid, oFlag);
		if (res != Core::J_FILE_IO_RESULT::SUCCESS)
			return res;

		if (tool.CanUseJSon())
		{
			bool temp;
			if (!Core::JJSon::LoadData(GetJSonHandle(tool), temp, Core::JFileConstant::GetActivatedSymbol()))
				return Core::J_FILE_IO_RESULT::FAIL_INVALID_DATA;
			isActivated = temp;
		}
		else
		{
			std::string guide;
			GetInputStream(tool) >> guide >> isActivated;
		} 
		return Core::J_FILE_IO_RESULT::SUCCESS;
	}
	Core::J_FILE_IO_RESULT JObjectFileIOHelper::_StoreHasIden(JFileIOTool& tool, Core::JIdentifier* iden, const std::string& guide, const bool applyUncopyableSymbol)
	{
		return JFileIOHelper::StoreHasIden(tool, iden, guide, applyUncopyableSymbol);
	} 
	JUserPtr<Core::JIdentifier> JObjectFileIOHelper::_LoadHasIden(JFileIOTool& tool, const std::string& key, const bool applyUncopyableSymbol)
	{
		TryAgainLoadObjIfFailPtr ptr = [](Core::JTypeInfo* info, size_t objGuid) 
			->  JUserPtr<Core::JIdentifier>
		{
			if (info->IsChildOf<JResourceObject>())
				return _JResourceManager::Instance().TryGetResourceUser(*info, objGuid);
			else
				return nullptr;
		}; 

		return LoadHasIden(tool, key, ptr, applyUncopyableSymbol);
	}
}