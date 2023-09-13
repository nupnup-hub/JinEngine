#pragma once 
#include"../../Gui/JGuiType.h"
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Func/Functor/JFunctor.h"
#include"../../../Core/Math/JVector.h"
#include<vector>

namespace JinEngine
{
	namespace Editor
	{  
		class JDockUpdateHelper
		{
		public:
			struct UpdateData
			{
			public: 
				std::unique_ptr<Core::JBindHandleBase> rollbackBind = nullptr; 
			};
		public:
			virtual bool IsLastWindow()const noexcept = 0;
			virtual bool IsLastDock()const noexcept = 0;
			virtual bool IsLockSplitAcitvated()const noexcept = 0;
			virtual bool IsLockOverAcitvated()const noexcept = 0;
			virtual bool IsLockMove()const noexcept = 0;
		public:
			virtual void Update(UpdateData& updateData) = 0;
		};
	}
}