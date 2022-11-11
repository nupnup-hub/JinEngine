#include"JAvatar.h"

namespace JinEngine
{ 
	const std::vector<std::vector<JAvatarJointGuide>> JAvatar::jointGuide
	{
		// Body
		{
			{"Root","root",0}, {"Hips","pervis",1},{"Spine","spine_01",2},{"Chest","spine_02",3},{"Upper Chest","spine_03",4},

			{"Left Shoulder","clavicle_l",5},{"Left Upper Arm","upperarm_l",6},{"Left Lower Arm","lowerarm_l",7},{"Left Hand","hand_l",8},
			{"Left Lower Arm Twist","lowerarm_twist_01_l",24},{"Left Upper Arm Twist","upperarm_twist_01_l",25},

			{"Right Shoulder","clavicle_r",26},{"Right Upper Arm","upperarm_r",27},{"Right Lower Arm","lowerarm_r",28},{"Right Hand","hand_r",29},
			{"Right Lower Arm Twist","lowerarm_twist_01_r",45},{"Right Upper Arm Twist","upperarm_twist_01_r",46},

			{"Left Upper Leg","thigh_l", 47},{"Left Lower Leg","calf_l",48}, {"Left Calf Twist","calf_twist_01_l",49},{"Left Foot","foot_l",50},
			{"Left Toes","ball_l",51},{"Left Thigh Twist","thigh_twist_01_l",52},

			{"Right Upper Leg","thigh_r",53},{"Right Lower Leg","calf_r",54}, {"Right Calf Twist","calf_twist_01_r",55},{"Right Foot","foot_r",56},
			{"Right Toes","ball_r",57},{"Right Thigh Twist","thigh_twist_01_r",58}
		},
		// Head
		{
			{"Neck","neck_01", 59},{"Head","head",60},{"Left Eye","Eye_l",61},{"Right Eye","Eye_r",62},{"Jaw","Mouth",63},
		},
		//Left Hand
		{
			{"Left Thumb Proximal","thumb_01_l",21},{"Left Thumb Intermediate", "thumb_02_l",22},{"Left Thumb Distal","thumb_03_l",23},
			{"Left Index Proximal","index_01_l",9},{"Left Index Intermediate","index_02_l",10},{"Left Index Distal","index_03_l",11},
			{"Left Middle Proximal","middle_01_l",12},{"Left Middle Intermediate","middle_02_l",13},{"Left Middle Distal","middle_03_l",14},
			{"Left Ring Proximal","ring_01_l",18},{"Left Ring Intermediate","ring_02_l",19},{"Left Ring Distal","ring_03_l",20},
			{"Left Little Proximal","pinky_01_l",15},{"Left Little Intermediate","pinky_02_l",16},{"Left Little Distal","pinky_03_l",17},
		},
		//Right Hand
		{
			{"Right Thumb Proximal","thumb_01_r",42},{"Right Thumb Intermediate","thumb_02_r",43},{"Right Thumb Distal","thumb_03_r",44},
			{"Right Index Proximal","index_01_r",30},{"Right Index Intermediate","index_02_r",31},{"Right Index Distal","index_03_r",32},
			{"Right Middle Proximal","middle_01_r",33},{"Right Middle Intermediate","middle_02_r",34},{"Right Middle Distal","middle_03_r",35},
			{"Right Ring Proximal","ring_01_r",39},{"Right Ring Intermediate","ring_02_r",40},{"Right Ring Distal","ring_03_r",41},
			{"Right Little Proximal","pinky_01_r",36},{"Right Little Intermediate","pinky_02_r",37},{"Right Little Distal","pinky_03_r",38},
		}
	};
	const std::vector<std::string> JAvatar::tabName
	{
		"Body", "Head", "Left Hand", "Right Hand"
	};
	const std::vector<uint8> JAvatar::jointReferenceParent
	{
		//Name : root 
		//Index : 0
		255,
		//Name : pelvis 
		//Index : 1
		0,
		//Name : spine_01
		//Index : 2
		1,
		// Name : spine_02
		//Index : 3
		2,
		//Name : spine_03
		//Index : 4
		3,
		//Name : clavicle_l
		//Index : 5
		4,
		//Name : upperarm_l
		//Index : 6
		5,
		//Name : lowerarm_l
		//Index : 7
		6,
		//Name : hand_l
		//Index : 8
		7,
		//Name : index_01_l
		//Index : 9
		8,
		//Name : index_02_l
		//Index : 10
		9,
		//Name : index_03_l
		//Index : 11
		10,
		//Name : middle_01_l
		//Index : 12
		8,
		//Name : middle_02_l
		//Index : 13
		12,
		//Name : middle_03_l
		//Index : 14
		13,
		//Name : pinky_01_l
		//Index : 15
		8,
		//Name : pinky_02_l
		//Index : 16
		15,
		//Name : pinky_03_l
		//Index : 17
		16,
		//Name : ring_01_l
		//Index : 18
		8,
		//Name : ring_02_l
		//Index : 19
		18,
		//Name : ring_03_l
		//Index : 20
		19,
		//Name : thumb_01_l
		//Index : 21
		8,
		//Name : thumb_02_l
		//Index : 22
		21,
		//Name : thumb_03_l
		//Index : 23
		22,
		//Name : lowerarm_twist_01_l
		//Index : 24
		7,
		//Name : upperarm_twist_01_l
		//Index : 25
		6,

		//Name : clavicle_r
		//Index : 26
		4,
		//Name : upperarm_r
		//Index : 27
		26,
		//Name : lowerarm_r
		//Index : 28
		27,
		//Name : hand_r
		//Index : 29
		28,
		//Name : index_01_r
		//Index : 30
		29,
		//Name : index_02_r
		//Index : 31
		30,
		//Name : index_03_r
		//Index : 32
		31,
		//Name : middle_01_l
		//Index : 33
		29,
		//Name : middle_02_l
		//Index : 34
		33,
		//Name : middle_03_l
		//Index : 35
		34,
		//Name : pinky_01_l
		//Index : 36
		29,
		//Name : pinky_02_l
		//Index : 37
		36,
		//Name : pinky_03_l
		//Index : 38
		37,
		//Name : ring_01_l
		//Index : 39
		29,
		//Name : ring_02_l
		//Index : 40
		39,
		//Name : ring_03_l
		//Index : 41
		40,
		//Name : thumb_01_l
		//Index : 42
		29,
		//Name : thumb_02_l
		//Index : 43
		42,
		//Name : thumb_03_l
		//Index : 44
		43,
		//Name : lowerarm_twist_01_l
		//Index : 45
		28,
		//Name : upperarm_twist_01_l
		//Index : 46
		27,

		//Name : thigh_l
		//Index : 47
		1,
		//Name : calf_l
		//Index : 48
		47,
		//Name : calf_twist_01_l
		//Index : 49
		48,
		//Name : foot_l
		//Index : 50
		48,
		//Name : ball_l
		//Index : 51
		50,
		//Name : thigh_twist_01_l
		//Index : 52
		47,

		//Name : thigh_r
		//Index : 53
		1,
		//Name : calf_r
		//Index : 54
		53,
		//Name : calf_twist_01_r
		//Index : 55
		54,
		//Name : foot_r
		//Index : 56
		54,
		//Name : ball_r
		//Index : 57
		56,
		//Name : thigh_twist_01_r
		//Index : 58
		53,

		//Name : neck_01
		//Index : 59
		4,
		//Name : head
		//Index : 60
		59,
		//Name : Eye_l
		//Index : 61
		60,
		//Name : Eye_r
		//Index : 62
		60,
		//Name : Mouth
		//Index : 63
		60,
	};
	const std::unordered_map<uint8, std::vector<uint8>> JAvatar::jointReferenceChildren
	{
		{0, {1}}, {1, {2, 47, 53}}, {2,{3}}, {3,{4}}, {4,{5, 26, 59}},

		{5,{6}},
		{6,{7,25}},	{7,{8,24}}, {8,{9, 12, 15, 18, 21}}, {9,{10}}, {10, {11}},
		{12,{13}}, {13, {14}}, {15,{16}}, {16, {17}}, {18,{19}}, {19, {20}},
		{21,{22}}, {22, {23}},

		{26,{27}},
		{27,{28,46}}, {28,{29,45}}, {29,{30, 33, 36, 39, 42}}, {30,{31}}, {31, {32}},
		{33,{34}}, {34, {35}}, {36,{37}}, {37, {38}}, {39,{40}}, {40, {41}},
		{42,{43}}, {43, {44}},

		{47, {48,52}}, {48, {49,50}}, {50, {51}},

		{53, {54, 58}}, {54, {55,56}}, {56, {57}},

		{59, {60}},{60, {61,62,63}},
	};
	const std::unordered_map<J_AVATAR_JOINT, uint8> JAvatar::jointReferenceMap
	{
		{J_AVATAR_JOINT::ROOT, 0}, {J_AVATAR_JOINT::HIPS, 1}, {J_AVATAR_JOINT::SPINE, 2},
		{J_AVATAR_JOINT::CHEST, 3}, {J_AVATAR_JOINT::UPPER_CHEST, 4},

		{J_AVATAR_JOINT::LEFT_SHOULDER, 5}, {J_AVATAR_JOINT::LEFT_UPPER_ARM, 6}, {J_AVATAR_JOINT::LEFT_LOWER_ARM, 7},
		{J_AVATAR_JOINT::LEFT_HAND, 8}, {J_AVATAR_JOINT::LEFT_LOWER_ARM_TWIST, 24},{J_AVATAR_JOINT::LEFT_UPPER_ARM_TWIST, 25},

		{J_AVATAR_JOINT::RIGHT_SHOULDER, 26}, {J_AVATAR_JOINT::RIGHT_UPPER_ARM, 27}, {J_AVATAR_JOINT::RIGHT_LOWER_ARM, 28},
		{J_AVATAR_JOINT::RIGHT_HAND, 29}, {J_AVATAR_JOINT::RIGHT_LOWER_ARM_TWIST, 45},{J_AVATAR_JOINT::RIGHT_UPPER_ARM_TWIST, 46},

		{J_AVATAR_JOINT::LEFT_UPPER_LEG, 47}, {J_AVATAR_JOINT::LEFT_LOWER_LEG, 48}, {J_AVATAR_JOINT::LEFT_CALF_TWIST, 49},
		{J_AVATAR_JOINT::LEFT_FOOT, 50}, {J_AVATAR_JOINT::LEFT_TOES, 51},{J_AVATAR_JOINT::LEFT_THIGH_TWIST, 52},

		{J_AVATAR_JOINT::RIGHT_UPPER_LEG, 53}, {J_AVATAR_JOINT::RIGHT_LOWER_LEG, 54}, {J_AVATAR_JOINT::RIGHT_CALF_TWIST, 55},
		{J_AVATAR_JOINT::RIGHT_FOOT, 56}, {J_AVATAR_JOINT::RIGHT_TOES, 57},{J_AVATAR_JOINT::RIGHT_THIGH_TWIST, 58},

		{J_AVATAR_JOINT::NECK, 59}, {J_AVATAR_JOINT::HEAD, 60},
		{J_AVATAR_JOINT::LEFT_EYE, 61},{J_AVATAR_JOINT::RIGHT_EYE, 62}, {J_AVATAR_JOINT::JAW, 63},

		{J_AVATAR_JOINT::LEFT_THUMB_PROXIMAL, 21},{J_AVATAR_JOINT::LEFT_THUMB_INTERMEDIATE, 22}, {J_AVATAR_JOINT::LEFT_THUMB_DISTAL, 23},
		{J_AVATAR_JOINT::LEFT_INDEX_PROXIMAL, 9},{J_AVATAR_JOINT::LEFT_INDEX_INTERMEDIATE, 10}, {J_AVATAR_JOINT::LEFT_INDEX_DISTAL, 11},
		{J_AVATAR_JOINT::LEFT_MIDDLE_PROXIMAL, 12},{J_AVATAR_JOINT::LEFT_MIDDLE_INTERMEDIATE, 13}, {J_AVATAR_JOINT::LEFT_MIDDLE_DISTAL, 14},
		{J_AVATAR_JOINT::LEFT_RING_PROXIMAL, 18},{J_AVATAR_JOINT::LEFT_RING_INTERMEDIATE, 19}, {J_AVATAR_JOINT::LEFT_RING_DISTAL, 20},
		{J_AVATAR_JOINT::LEFT_LITTLE_PROXIMAL, 15},{J_AVATAR_JOINT::LEFT_LITTLE_INTERMEDIATE, 16}, {J_AVATAR_JOINT::LEFT_LITTLE_DISTAL, 17},

		{J_AVATAR_JOINT::RIGHT_THUMB_PROXIMAL, 42},{J_AVATAR_JOINT::RIGHT_THUMB_INTERMEDIATE, 43}, {J_AVATAR_JOINT::RIGHT_THUMB_DISTAL, 44},
		{J_AVATAR_JOINT::RIGHT_INDEX_PROXIMAL, 30},{J_AVATAR_JOINT::RIGHT_INDEX_INTERMEDIATE, 31}, {J_AVATAR_JOINT::RIGHT_INDEX_DISTAL, 32},
		{J_AVATAR_JOINT::RIGHT_MIDDLE_PROXIMAL, 33},{J_AVATAR_JOINT::RIGHT_MIDDLE_INTERMEDIATE, 34}, {J_AVATAR_JOINT::RIGHT_MIDDLE_DISTAL, 35},
		{J_AVATAR_JOINT::RIGHT_RING_PROXIMAL, 39},{J_AVATAR_JOINT::RIGHT_RING_INTERMEDIATE, 40}, {J_AVATAR_JOINT::RIGHT_RING_DISTAL, 41},
		{J_AVATAR_JOINT::RIGHT_LITTLE_PROXIMAL, 36},{J_AVATAR_JOINT::RIGHT_LITTLE_INTERMEDIATE, 37}, {J_AVATAR_JOINT::RIGHT_LITTLE_DISTAL, 38},
	};
	uint8 JAvatar::FindReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed)noexcept
	{
		if (st < ed)
			return JSkeletonFixedData::incorrectJointIndex;

		auto stData = jointReferenceMap.find(st);
		if (jointReference[stData->second] != JSkeletonFixedData::incorrectJointIndex)
			return stData->second;

		auto edData = jointReferenceMap.find(ed);
		if (jointReference[edData->second] == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;

		uint nowJointRefIndex = jointReferenceParent[stData->second];
		while (nowJointRefIndex != edData->second && nowJointRefIndex != JSkeletonFixedData::incorrectJointIndex)
		{
			if (jointReference[nowJointRefIndex] != JSkeletonFixedData::incorrectJointIndex)
				return nowJointRefIndex;

			nowJointRefIndex = jointReferenceParent[nowJointRefIndex];
		}

		if (nowJointRefIndex == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;
		else
			return edData->second;
	}
	uint8 JAvatar::FindReferenceIndexEndToRoot(const uint8 st, const uint8 ed)noexcept
	{
		if (jointReference[st] != JSkeletonFixedData::incorrectJointIndex)
			return st;

		if (jointReference[ed] == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;

		uint nowJointRefIndex = jointReferenceParent[st];
		while (nowJointRefIndex != ed && nowJointRefIndex != JSkeletonFixedData::incorrectJointIndex)
		{
			if (jointReference[nowJointRefIndex] != JSkeletonFixedData::incorrectJointIndex)
				return nowJointRefIndex;

			nowJointRefIndex = jointReferenceParent[nowJointRefIndex];
		}

		if (nowJointRefIndex == JSkeletonFixedData::incorrectJointIndex)
			return JSkeletonFixedData::incorrectJointIndex;
		else
			return nowJointRefIndex;
	}
	void JAvatar::FindCommonReferenceIndexEndToRoot(const J_AVATAR_JOINT st, const J_AVATAR_JOINT ed, const JAvatar* target, uint8& srcIndex, uint8& tarIndex)noexcept
	{
		srcIndex = FindReferenceIndexEndToRoot(st, ed);
		tarIndex = FindReferenceIndexEndToRoot(st, ed);

		if (srcIndex > tarIndex)
			srcIndex = tarIndex;
		else if (srcIndex < tarIndex)
			tarIndex = srcIndex;
	}
}