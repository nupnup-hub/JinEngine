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
		static Core::J_FILE_IO_RESULT StoreObjectIden(std::wofstream& stream, JObject* obj);
		static Core::J_FILE_IO_RESULT LoadObjectIden(std::wifstream& stream, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
		static Core::J_FILE_IO_RESULT LoadObjectIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag);
	public:
		static Core::J_FILE_IO_RESULT StoreComponentIden(std::wofstream& stream, JComponent* comp);
		static Core::J_FILE_IO_RESULT LoadComponentIden(std::wifstream& stream, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
		static Core::J_FILE_IO_RESULT LoadComponentIden(std::wifstream& stream, _Out_ std::wstring& oName, _Out_ size_t& oGuid, _Out_ J_OBJECT_FLAG& oFlag, _Out_ bool& isActivated);
	public:
		static Core::J_FILE_IO_RESULT _StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden);
		static Core::J_FILE_IO_RESULT _StoreHasIden(std::wofstream& stream, Core::JIdentifier* iden, const std::wstring& guiSymbol);
		/**
		* can use identifier child class
		* but try again ptr only valid object child class
		*/
		static JUserPtr<Core::JIdentifier> _LoadHasIden(std::wifstream& stream);
		template<typename T>
		static JUserPtr<T> _LoadHasIden(std::wifstream& stream)
		{
			return JUserPtr<T>::ConvertChild(_LoadHasIden(stream));
		} 
	};
}