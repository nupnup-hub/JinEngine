#include"JEditorDiagramNode.h"

namespace JinEngine
{
	namespace Editor
	{
		JEditorDiagramNode::JEditorDiagramNode(float posX, float posY)
			:posX(posX), posY(posY)
		{}
		JEditorDiagramNode::~JEditorDiagramNode() {}

		float JEditorDiagramNode::GetPosX()const noexcept
		{
			return posX;
		}
		float JEditorDiagramNode::GetPosY()const noexcept
		{
			return posY;
		}
		void JEditorDiagramNode::SetPos(const float newPosX, const float newPosY)noexcept
		{
			posX = newPosX;
			posY = newPosY;
		}
		void JEditorDiagramNode::SetPosX(const float newPosX)noexcept
		{
			posX = newPosX;
		}
		void JEditorDiagramNode::SetPosY(const float newPosY)noexcept
		{
			posY = newPosY;
		}
	}
}