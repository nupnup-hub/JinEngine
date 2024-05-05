#pragma once
#include"../JGuiPrivateData.h"  
#include"../../../../Core/Math/JVector.h"
#include"../../../../Core/Platform/JLanguageType.h"
#include"../../../../../ThirdParty/imgui/imgui.h"
 
namespace JinEngine
{
	namespace Editor
	{
		struct JImGuiPrivateData final: public JGuiPrivateData
		{
		private:
			using FontMap = std::unordered_map<J_GUI_FONT_TYPE, std::unordered_map<Core::J_LANGUAGE_TYPE, ImFont*>>;
		public:
			FontMap fontMap;
			J_GUI_FONT_TYPE fontType;
		public: 
			//default color cashing 
			JVector4<float> colors[ImGuiCol_COUNT]; 
		public:
			J_GUI_TYPE GetGuiType()const noexcept final;
		};
	}
}