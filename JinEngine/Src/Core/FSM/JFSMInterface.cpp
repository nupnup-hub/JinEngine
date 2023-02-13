#include"JFSMInterface.h"
#include"../File/JFileConstant.h"
#include"../../Utility/JCommonUtility.h"  
#include<fstream>

namespace JinEngine
{
	namespace Core
	{
		JFSMInterface::JFSMIdentifierInitData::JFSMIdentifierInitData(const std::wstring& name, const size_t guid)
			:name(name), guid(guid)
		{} 
		void JFSMInterface::Destroy(JFSMInterface* fsmInterface)
		{
			if (fsmInterface == nullptr)
				return;

			fsmInterface->Clear();
			fsmInterface->RemoveInstance();
		}
		JFSMInterface::JFSMInterface(const JFSMIdentifierInitData& initData)
			:JIdentifier(initData.name, initData.guid)
		{}
	};
}
