#include"EditorDiagramNode.h"

namespace JinEngine
{
	EditorDiagramNode::EditorDiagramNode(float posX, float posY)
		:posX(posX), posY(posY)
	{}
	EditorDiagramNode::~EditorDiagramNode(){}

	float EditorDiagramNode::GetPosX()const noexcept
	{
		return posX;
	}
	float EditorDiagramNode::GetPosY()const noexcept
	{
		return posY;
	}
	void EditorDiagramNode::SetPos(const float newPosX, const float newPosY)noexcept
	{
		posX = newPosX;
		posY = newPosY;
	}
	void EditorDiagramNode::SetPosX(const float newPosX)noexcept
	{
		posX = newPosX;
	}
	void EditorDiagramNode::SetPosY(const float newPosY)noexcept
	{
		posY = newPosY;
	}
}