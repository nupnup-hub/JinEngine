#pragma once
namespace JinEngine
{
	namespace Editor
	{
		class JEditorDiagramNode
		{
		private:
			float posX = 0;
			float posY = 0;
		public:
			JEditorDiagramNode(float posX = 0, float posY = 0);
			~JEditorDiagramNode();

			JEditorDiagramNode(const JEditorDiagramNode& rhs) = delete;
			JEditorDiagramNode& operator=(const JEditorDiagramNode& rhs) = delete;

			float GetPosX()const noexcept;
			float GetPosY()const noexcept;

			void SetPos(const float newPosX, const float newPosY)noexcept;
			void SetPosX(const float newPosX)noexcept;
			void SetPosY(const float newPosY)noexcept;
		};
	}
}