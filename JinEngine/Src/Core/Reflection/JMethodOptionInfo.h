#pragma once 

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Pointer, Pointer ptr> class JMethodInfoRegisterHelper;
		class JMethodInfo;

		class JMethodOptionInfo
		{
		private:
			friend class JMethodInfo;
			template<typename Type, typename Pointer, Pointer ptr> friend class JMethodInfoRegisterHelper;
			friend std::unique_ptr<JMethodOptionInfo>::deleter_type;
		private:
			JMethodOptionInfo() = default; 
		};
	}
}