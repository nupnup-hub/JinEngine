#pragma once 
#include"JGraphicInfo.h"
#include"JGraphicOption.h"

namespace JinEngine
{
	namespace Graphic
	{   
		struct JGraphicInfoChangedSet;
		struct JGraphicOptionChangedSet;
		class JGraphicSubClassInterface
		{
		private:
			friend class JGraphic;
		public: 
			virtual ~JGraphicSubClassInterface() = default;
		protected:
			const JGraphicInfo& GetGraphicInfo()const noexcept;
			const JGraphicOption& GetGraphicOption()const noexcept;
		private:
			virtual bool HasDependency(const JGraphicInfo::TYPE type)const noexcept;
			virtual bool HasDependency(const JGraphicOption::TYPE type)const noexcept;
		private:
			virtual void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set);
			virtual void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set);
		};
	}
}