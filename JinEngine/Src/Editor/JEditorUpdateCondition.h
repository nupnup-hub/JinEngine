#pragma once
namespace JinEngine
{
	namespace Editor
	{
		struct JEditorWindowUpdateCondition
		{
		public:
			bool canClickWindow = true;
		public:
			JEditorWindowUpdateCondition() = default;
			JEditorWindowUpdateCondition(const bool canClickWindow);
		};

		struct JEditorPageUpdateCondition
		{
		public:
			bool canClickPage = true;
		public:
			JEditorPageUpdateCondition() = default;
			JEditorPageUpdateCondition(const bool canClickPage);
		public:
			JEditorWindowUpdateCondition CreateWindowCondition()const noexcept;
		};
	}
}