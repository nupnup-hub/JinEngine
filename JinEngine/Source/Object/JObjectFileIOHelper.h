#pragma once
#include"JObject.h"
#include"../Core/File/JFileIOHelper.h"
namespace JinEngine
{ 
	class JComponent;
	class JObjectFileIOHelper : public JFileIOHelper
	{ 
	private: 
		using JFileIOHelper::StoreHasIden;
		using JFileIOHelper::LoadHasIden;
	public:
		static Core::J_FILE_IO_RESULT StoreObjectIden(JFileIOTool& tool, JObject* obj);
		static Core::J_FILE_IO_RESULT LoadObjectIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
		static Core::J_FILE_IO_RESULT LoadObjectIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
	public:
		static Core::J_FILE_IO_RESULT StoreComponentIden(JFileIOTool& tool, JComponent* comp);
		static Core::J_FILE_IO_RESULT LoadComponentIden(JFileIOTool& tool, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
		static Core::J_FILE_IO_RESULT LoadComponentIden(JFileIOTool& tool, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
	public:
		static Core::J_FILE_IO_RESULT _StoreHasIden(JFileIOTool& tool, Core::JIdentifier* iden, const std::string& guide, const bool applyUncopyableSymbol = false);
		/**
		* can use identifier child class
		* but try again ptr only valid object child class
		*/
		static JUserPtr<Core::JIdentifier> _LoadHasIden(JFileIOTool& tool, const std::string& key = "", const bool applyUncopyableSymbol = false);
		template<typename T>
		static JUserPtr<T> _LoadHasIden(JFileIOTool& tool, const std::string& key = "", const bool applyUncopyableSymbol = false)
		{
			return JUserPtr<T>::ConvertChild(_LoadHasIden(tool, key));
		} 
	};
}