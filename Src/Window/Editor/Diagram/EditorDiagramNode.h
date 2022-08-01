#pragma once
namespace JinEngine
{
	struct EditorDiagramNode
	{
	private:
		float posX = 0;
		float posY = 0;
	public:
		EditorDiagramNode(float posX = 0, float posY = 0);
		~EditorDiagramNode();

		EditorDiagramNode(const EditorDiagramNode& rhs) = delete;
		EditorDiagramNode& operator=(const EditorDiagramNode& rhs) = delete;

		float GetPosX()const noexcept;
		float GetPosY()const noexcept;

		void SetPos(const float newPosX, const float newPosY)noexcept;
		void SetPosX(const float newPosX)noexcept;
		void SetPosY(const float newPosY)noexcept;
	};
}