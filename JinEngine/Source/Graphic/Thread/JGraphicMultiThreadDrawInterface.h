#pragma once
#include"../DataSet/JGraphicDataSet.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDrawHelper;
		class JGraphicMultiThreadDrawInterface
		{
		public:
			virtual ~JGraphicMultiThreadDrawInterface() = default;
		public:
			virtual bool HasPreprocessing()const noexcept = 0;	//if true call BeginDraw before draw
			virtual bool HasPostprocessing()const noexcept = 0;  //if true call EndDraw after draw
		public:
			//for multi thread
			//clear rtv dsv and set resource state
			virtual void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper) = 0;
			//for multi thread
			//set resource state
			virtual void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper) = 0;
		};
	}
}