#include"JFbxUtility.h"
#include"../../../../Animation/Joint.h"

namespace JinEngine
{
	namespace Core
	{
		JFbxJoint::JFbxJoint()
			: animation(nullptr), node(nullptr), parentIndex(-1)
		{
			globalBindPositionInverse.SetIdentity();
		}
		JFbxJoint::~JFbxJoint()
		{
			while (animation)
			{
				JFbxKeyFrame* temp = animation->next;
				delete animation;
				animation = temp;
			}
		} 
	}
}