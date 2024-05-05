#pragma once
#include"../Gui/JGuiType.h"
#include"../../Core/JCoreEssential.h"
#include"../../Core/Reflection/JReflection.h" 
#include"../../Core/Interface/JTreeInterface.h"  
#include"../../Core/Geometry/JBBox.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorMouseDragBox;
		class JEditorSelectableStructure
		{ 
		private:
			//update per begin
			std::vector<Core::JBBox2D> lastUpdated;
			bool useFrame = false;
			bool useMultiColor = false;
			bool applyRounded = false;
			bool isRecordUpdateData = false;
		private:
			//optional
			JVector4F deltaColor = JVector4F::Zero();
			float frameThickness = 1;
		public:
			void Begin(const bool useFrame, const bool useMultiColor, const bool applyRounded, const bool isRecordUpdateData = false);
			void End();
		public:
			bool DisplaySelectable(const std::string& label, 
				J_GUI_SELECTABLE_FLAG_ flags, 
				const bool isFocus, 
				const bool isActivated, 
				const bool isSelected,
				const JVector2F size,
				const JVector4F color,
				const JVector4F frameColor = JVector4F::Zero()); 
		public:
			J_GUI_SELECTABLE_FLAG_ GetBaseFlag()const noexcept;
			std::vector<Core::JBBox2D> GetLastUpdated()const noexcept;
		public:
			void SetDeltaColor(const JVector4F& col)noexcept;
			void SetFrameThickness(const float thickness)noexcept;
		}; 
	}
}