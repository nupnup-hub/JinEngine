#include"JKdTreeNode.h" 
#include"../../../Object/GameObject/JGameObject.h" 
#include"../../../Object/GameObject/JGameObjectFactoryUtility.h"
#include"../../../Object/Component/RenderItem/JRenderItem.h"
#include"../../../Object/Component/Transform/JTransform.h" 
#include"../../../Editor/Utility/JEditorBinaryTreeView.h"
#include"../../../Utility/JCommonUtility.h" 
#include"../../../Utility/JVectorExtend.h" 
#include"../../../Utility/JMathHelper.h"

//Test
//#include"../../../Object/Resource/JResourceManager.h"

using namespace DirectX;
namespace JinEngine
{
	namespace Core
	{ 
		JKdTreeNode::JKdTreeNode(const uint nodeNumber, const J_KDTREE_NODE_TYPE nodeType, const DirectX::BoundingBox& bbox, JKdTreeNode* parent)
			:nodeNumber(nodeNumber), nodeType(nodeType), bbox(bbox), parent(parent)
		{  
			if (parent != nullptr)
			{
				if (parent->left == nullptr)
					parent->left = this;
				else
					parent->right = this;				 
			}
			//else
			//	JKdTreeNode::nodeType = J_KDTREE_NODE_TYPE::ROOT;
		}
		JKdTreeNode::~JKdTreeNode() {}
		void JKdTreeNode::CreateDebugGameObject(JGameObject* parent, bool onlyLeafNode)noexcept
		{
			if (debugGameObject == nullptr)
			{
				if (nodeType != J_KDTREE_NODE_TYPE::LEAF && onlyLeafNode)
					return;

				if (nodeType != J_KDTREE_NODE_TYPE::LEAF)
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX, J_DEFAULT_MATERIAL::DEBUG_LINE_RED);
				else
					debugGameObject = JGFU::CreateDebugGameObject(*parent, OBJECT_FLAG_EDITOR_OBJECT, J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX, J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN);

				debugGameObject->GetTransform()->SetScale(XMFLOAT3(bbox.Extents.x * 2, bbox.Extents.y * 2, bbox.Extents.z * 2));
				debugGameObject->GetTransform()->SetPosition(bbox.Center);
			}
		}
		void JKdTreeNode::DestroyDebugGameObject()noexcept
		{
			if (debugGameObject != nullptr)
			{
				JGameObject::BeginDestroy(debugGameObject); 
				debugGameObject = nullptr;
			}
		}
		void JKdTreeNode::Clear()noexcept
		{ 
			if (innerGameObject.size() > 0)
				SetVisible();
			innerGameObject.clear();
			parent = nullptr;
			left = nullptr;
			right = nullptr;
			DestroyDebugGameObject();
		}
		void JKdTreeNode::Culling(const JCullingFrustum& camFrustum, J_CULLING_FLAG flag)noexcept
		{
			J_CULLING_RESULT res = camFrustum.IsBoundingBoxIn(bbox, flag);
			if (res == J_CULLING_RESULT::CONTAIN)
				SetVisible();
			else if (res == J_CULLING_RESULT::DISJOINT)
				SetInVisible();
			else
			{
				if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
					SetVisible();
				else
				{
					left->Culling(camFrustum, flag);
					right->Culling(camFrustum, flag);
				}
			}
		}
		void JKdTreeNode::Culling(const DirectX::BoundingFrustum& camFrustum)noexcept
		{
			ContainmentType res = camFrustum.Contains(bbox);
			if (res == ContainmentType::CONTAINS)
				SetVisible();
			else if (res == ContainmentType::DISJOINT)
				SetInVisible();
			else
			{
				if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
					SetVisible();
				else
				{
					left->Culling(camFrustum);
					right->Culling(camFrustum);
				}
			}
		}
		void JKdTreeNode::OffCulling()noexcept
		{
			SetVisible();
		}
		bool JKdTreeNode::IsLeftNode()const noexcept
		{
			return parent != nullptr ? parent->left->nodeNumber == nodeNumber : false;
		}
		bool JKdTreeNode::IsChildNode(const uint pNodeNumber)const noexcept
		{
			JKdTreeNode* nowParent = parent;
			while (nowParent != nullptr)
			{
				if (nowParent->nodeNumber == pNodeNumber)
					return true;
			}
			return false;
		}
		uint JKdTreeNode::GetNodeNumber()const noexcept
		{
			return nodeNumber;
		}
		uint JKdTreeNode::GetLeftNumberEnd()const noexcept
		{
			if (left == nullptr)
				return nodeNumber;
			else
				return left->FindRightLeafNode()->nodeNumber;
		}
		uint JKdTreeNode::GetRightNumberEnd()const noexcept
		{
			if (right == nullptr)
				return nodeNumber;
			else
				return right->FindRightLeafNode()->nodeNumber;
		}
		J_KDTREE_NODE_TYPE JKdTreeNode::GetNodeType()const noexcept
		{
			return nodeType;
		}
		DirectX::BoundingBox JKdTreeNode::GetBoundingBox()const noexcept
		{
			return bbox;
		}
		JKdTreeNode* JKdTreeNode::GetParentNode()noexcept
		{
			return parent;
		}
		JKdTreeNode* JKdTreeNode::GetLeftNode()const noexcept
		{
			return left;
		}
		JKdTreeNode* JKdTreeNode::GetRightNode()const noexcept
		{
			return right;
		}
		std::vector<JGameObject*> JKdTreeNode::GetInnerGameObject()const noexcept
		{
			return innerGameObject;
		}
		std::vector<JGameObject*> JKdTreeNode::GetAreaInnerGameObject()const noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return innerGameObject;
			else
			{ 			
				std::vector<JGameObject*> leftRes = left->GetAreaInnerGameObject();
				std::vector<JGameObject*> rightRes = right->GetAreaInnerGameObject();
				leftRes.insert(leftRes.end(), rightRes.begin(), rightRes.end());
				return leftRes;
			}
		}
		uint JKdTreeNode::GetInnerGameObjectCount()const noexcept
		{
			return (uint)innerGameObject.size();
		}
		uint JKdTreeNode::GetDepth()const noexcept
		{
			uint depth = 0;
			JKdTreeNode* nowParent = parent;
			while (nowParent != nullptr)
			{
				nowParent = nowParent->parent;
				++depth;
			}
			return depth;
		}
		void JKdTreeNode::GetAlignLeafNode(const JVector3<float>& pos, std::vector<JKdTreeNode*>& alignNode, uint& index) noexcept
		{
			if (index >= alignNode.size())
				return;

			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				alignNode[index++] = this;
			else
			{ 
				float centerFactor = 0;
				if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
					centerFactor = bbox.Center.x;
				else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
					centerFactor = bbox.Center.y;
				else
					centerFactor = bbox.Center.z;

				if (pos[(int)splitType] > centerFactor)
				{
					right->GetAlignLeafNode(pos, alignNode, index);
					left->GetAlignLeafNode(pos, alignNode, index);
				}
				else
				{
					left->GetAlignLeafNode(pos, alignNode, index);
					right->GetAlignLeafNode(pos, alignNode, index);
				}
			}
		}
		void JKdTreeNode::GetAlignLeafNode(const DirectX::BoundingFrustum& camFrustum, std::vector<JKdTreeNode*>& alignNode, uint& index)noexcept
		{
			if (index >= alignNode.size())
				return;

			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				alignNode[index++] = this;
			else
			{
				if (camFrustum.Contains(bbox) == ContainmentType::DISJOINT)
					return;

				bool isRight = false;
				if (splitType == J_KDTREE_NODE_SPLIT_AXIS::X)
					isRight = camFrustum.Origin.x > bbox.Center.x;
				else if (splitType == J_KDTREE_NODE_SPLIT_AXIS::Y)
					isRight = camFrustum.Origin.y > bbox.Center.y;
				else
					isRight = camFrustum.Origin.z > bbox.Center.z;
		
				if (isRight)
				{
					right->GetAlignLeafNode(camFrustum, alignNode, index);
					left->GetAlignLeafNode(camFrustum, alignNode, index);
				}
				else
				{
					left->GetAlignLeafNode(camFrustum, alignNode, index);
					right->GetAlignLeafNode(camFrustum, alignNode, index);
				}
			}
		}
		void JKdTreeNode::SetNodeNumber(const uint newNodeNumber)noexcept
		{
			nodeNumber = newNodeNumber;
		} 
		void JKdTreeNode::SetSplitType(const uint newSplitType)noexcept
		{
			if (newSplitType == 0)
				splitType = J_KDTREE_NODE_SPLIT_AXIS::X;
			else if (newSplitType == 1)
				splitType = J_KDTREE_NODE_SPLIT_AXIS::Y;
			else
				splitType = J_KDTREE_NODE_SPLIT_AXIS::Z;
		}
		void JKdTreeNode::SetSplitType(const J_KDTREE_NODE_SPLIT_AXIS newSplitType)noexcept
		{
			splitType = newSplitType;
		}
		void JKdTreeNode::SetLeftNode(JKdTreeNode* newLeftNode)noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return;

			left = newLeftNode;
			if (left != nullptr)
				left->parent = this;
		}
		void JKdTreeNode::SetRightNode(JKdTreeNode* newRightNode)noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return;

			right = newRightNode;
			if (right != nullptr)
				right->parent = this;
		}
		bool JKdTreeNode::HasGameObject(const size_t guid)noexcept
		{ 
			return JCUtil::GetJIdenIndex(innerGameObject, guid) != JCUtil::searchFail;
		}
		void JKdTreeNode::AddInnerGameObject(JGameObject* newInnerGameObject)noexcept
		{
			innerGameObject.push_back(newInnerGameObject);
		}
		void JKdTreeNode::AddInnerGameObject(const std::vector<JGameObject*>& newInnerGameObject)noexcept
		{
			innerGameObject = newInnerGameObject;
		}
		void JKdTreeNode::RemoveInnerGameObject(const size_t guid)noexcept
		{
			int index = JCUtil::GetJIdenIndex(innerGameObject, guid);
			if (index != JCUtil::searchFail)
				innerGameObject.erase(innerGameObject.begin() + index);
		}
		void JKdTreeNode::StuffInnerGameObject(std::vector<JGameObject*>& objList, const uint& offset)
		{
			const uint innerCount = (uint)innerGameObject.size();
			for (uint i = 0; i < innerCount; ++i)
				objList[i + offset] = innerGameObject[i];
		}
		void JKdTreeNode::SetVisible()noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
					innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::VISIBLE);
			} 
			else
			{
				left->SetVisible();
				right->SetVisible();
			}
		}
		void JKdTreeNode::SetInVisible()noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
			{
				const uint innerCount = (uint)innerGameObject.size();
				for (uint i = 0; i < innerCount; ++i)
					innerGameObject[i]->GetRenderItem()->SetRenderVisibility(J_RENDER_VISIBILITY::INVISIBLE);
			}
			else
			{
				left->SetInVisible();
				right->SetInVisible();
			}
		}
		JKdTreeNode* JKdTreeNode::FindRightLeafNode() noexcept
		{
			if (nodeType == J_KDTREE_NODE_TYPE::LEAF)
				return this;
			else
				return right->FindRightLeafNode();
		}
		void JKdTreeNode::BuildDebugNode(Editor::JEditorBinaryTreeView& treeView)
		{
			if (nodeType == J_KDTREE_NODE_TYPE::ROOT)
			{
				treeView.BuildNode(std::to_string(nodeNumber));
				if (left != nullptr)
					left->BuildDebugNode(treeView);
				if (right != nullptr)
					right->BuildDebugNode(treeView);
			}
			else if (nodeType == J_KDTREE_NODE_TYPE::NODE)
			{
				treeView.BuildNode(std::to_string(nodeNumber));
				left->BuildDebugNode(treeView);
				right->BuildDebugNode(treeView);
				if (IsLeftNode())
					treeView.EndLeft();
				else
					treeView.EndRight();
			}
			else
			{
				std::wstring info;
				for (const auto& data : innerGameObject)
					info += data->GetName() + L"\n";
				treeView.BuildNode(std::to_string(nodeNumber), JCUtil::WstrToU8Str(info));
				if (IsLeftNode())
					treeView.EndLeft();
				else
					treeView.EndRight();
			}
		}
		//void JKdTreeNode::AddLog(const std::string& newNodeLog)noexcept
		//{
		//	nodeLog += newNodeLog;
		//}
	}
}