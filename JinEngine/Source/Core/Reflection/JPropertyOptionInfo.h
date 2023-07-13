#pragma once 

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyInfoRegister;
		template<typename Type, typename Field, typename Pointer, Pointer ptr> class JPropertyExInfoRegister;
		class JPropertyInfo;

		class JPropertyOptionInfo
		{ 
		private:
			friend class JPropertyInfo;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyExInfoRegister;
			friend std::unique_ptr<JPropertyOptionInfo>::deleter_type;
		private:
			JPropertyOptionInfo() = default; 
		};
	}
}