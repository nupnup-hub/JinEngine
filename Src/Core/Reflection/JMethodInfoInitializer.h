#pragma once 
#include"../JDataType.h"  
#include"JParameter.h"
#include"JMethodOptionInfo.h"

namespace JinEngine
{
	namespace Core
	{
		class JTypeInfo;
		class JMethodInfo;
		class JCallableBase;
		class JCallableHintBase;

		template<typename Ret, typename ...Param>
		class JMethodInfoInitializer
		{
		private:
			friend class JMethodInfo;
		private:
			constexpr static char fence = '@';
		private:
			std::string name;
			std::string identificationName;
			JParameterHint returnHint = CreateParameterHint<Ret>(typeid(Ret).name());
			std::vector<JParameterHint> parameterHint;
			JCallableBase* callHandle;
			JCallableHintBase* hintHandle;
			JTypeInfo* ownerType;
		private:
			JMethodOptionInfo* optionInfo;
		public:
			JMethodInfoInitializer(const std::string& name,
				const std::string& identificationName,
				const std::string& funcPtrName,
				JCallableBase& callHandle,
				JCallableHintBase& hintHandle,
				JTypeInfo& ownerType,
				JMethodOptionInfo& optionInfo)
				:name(name),
				identificationName(identificationName),
				callHandle(&callHandle),
				hintHandle(&hintHandle),
				ownerType(&ownerType),
				optionInfo(&optionInfo)
			{
				std::vector<std::string> nameVec;
				ClassifyParamName((uint)sizeof...(Param), nameVec, funcPtrName);
				CreateParameterHint<Param...>(std::make_index_sequence<sizeof...(Param)>(), parameterHint, nameVec);
			}
		};
	}
}