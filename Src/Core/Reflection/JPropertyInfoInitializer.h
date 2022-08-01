#pragma once    
#include<string>
#include"JParameter.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JPropertyInfo;
		class PropertyHandlerBase;
		template<typename Field>
		class JPropertyInfoInitializer
		{
			friend class JPropertyInfo;
		public:
			JPropertyInfoInitializer(const std::string& name, PropertyHandlerBase& handlerBase, JTypeInfo& jTypeInfo)
				:name(name),
				handlerBase(&handlerBase),
				ownerType(&jTypeInfo)
			{}
		private:

		private:
			const std::string name;
			JParameterHint fieldHint = CreateParameterHint<Field>(typeid(Field).name());
			PropertyHandlerBase* handlerBase;
			JTypeInfo* ownerType;
		};
	}
}