#pragma once     
#include"JParameter.h" 
#include"JPropertyOptionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JPropertyInfo;
		class JPropertyHandlerBase;
		template<typename Field>
		class JPropertyInfoInitializer
		{
		private:
			friend class JPropertyInfo;
		private:
			const std::string name;
			JParameterHint fieldHint = CreateParameterHint<Field>(typeid(Field).name());
			JPropertyHandlerBase* handlerBase;
			JTypeInfo* ownerType; 
			JPropertyOptionInfo* optionInfo;
		public:
			JPropertyInfoInitializer(const std::string& name,
				JPropertyHandlerBase& handlerBase, 
				JTypeInfo& jTypeInfo, 
				JPropertyOptionInfo& optionInfo)
				:name(name),
				handlerBase(&handlerBase),
				ownerType(&jTypeInfo),
				optionInfo(&optionInfo)
			{}
		};
	}
}