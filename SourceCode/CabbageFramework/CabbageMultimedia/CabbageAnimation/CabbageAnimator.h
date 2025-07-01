#pragma once


#include "CabbageFramework/CabbageAssets/ModelImporter.h"

#include "CabbageFramework/CabbageFoundation/CabbageFramework.h"


class CabbageAnimator
{
public:

	CabbageAnimator()
	{
		m_CurrentTime = 0.0;
		//m_CurrentAnimation = Animation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(ktm::fmat4x4::from_eye());
	}


	const std::vector<ktm::fmat4x4>& UpdateBoneAnimation(Animation* animation, float dt)
	{
		m_CurrentAnimation = animation;
		m_DeltaTime = dt;
		if (m_CurrentAnimation)
		{
			m_CurrentTime += (float)m_CurrentAnimation->m_TicksPerSecond * dt;
			m_CurrentTime = fmod(m_CurrentTime, (float)m_CurrentAnimation->m_Duration);
			CalculateBoneTransform(&m_CurrentAnimation->m_RootNode, ktm::fmat4x4::from_eye());
		}

		return m_FinalBoneMatrices;
	}


	/* Gets the current index on mKeyPositions to interpolate to based on
	the current animation time*/
	int GetPositionIndex(Bone* Bone, float animationTime)
	{
		for (int index = 0; index < Bone->m_NumPositions - 1; ++index)
		{
			if (animationTime < Bone->m_Positions[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return -1;
	}

	/* Gets the current index on mKeyRotations to interpolate to based on the
	current animation time*/
	int GetRotationIndex(Bone* Bone, float animationTime)
	{
		for (int index = 0; index < Bone->m_NumRotations - 1; ++index)
		{
			if (animationTime < Bone->m_Rotations[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return -1;
	}

	/* Gets the current index on mKeyScalings to interpolate to based on the
	current animation time */
	int GetScaleIndex(Bone* Bone, float animationTime)
	{
		for (int index = 0; index < Bone->m_NumScalings - 1; ++index)
		{
			if (animationTime < Bone->m_Scales[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return -1;
	}


	/* Gets normalized value for Lerp & Slerp*/
	float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		float scaleFactor = 0.0f;
		float midWayLength = animationTime - lastTimeStamp;
		float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	/*figures out which position keys to interpolate b/w and performs the interpolation
	and returns the translation matrix*/
	ktm::fmat4x4 InterpolatePosition(Bone* Bone, float animationTime)
	{
		if (1 == Bone->m_NumPositions)
			return ktm::translate3d(Bone->m_Positions[0].position);

		int p0Index = GetPositionIndex(Bone, animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(Bone->m_Positions[p0Index].timeStamp,
			Bone->m_Positions[p1Index].timeStamp, animationTime);
		ktm::fvec3 finalPosition = ktm::lerp(Bone->m_Positions[p0Index].position,
			Bone->m_Positions[p1Index].position, scaleFactor);
		return ktm::translate3d(finalPosition);
	}

	ktm::fmat4x4 InterpolateRotation(Bone* Bone, float animationTime)
	{
		if (1 == Bone->m_NumRotations)
		{
			ktm::fquat rotation = ktm::normalize(Bone->m_Rotations[0].orientation);
			return rotation.matrix4x4();
		}

		int p0Index = GetRotationIndex(Bone, animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(Bone->m_Rotations[p0Index].timeStamp,
			Bone->m_Rotations[p1Index].timeStamp, animationTime);
		ktm::fquat finalRotation = ktm::slerp(Bone->m_Rotations[p0Index].orientation,
			Bone->m_Rotations[p1Index].orientation, scaleFactor);
		finalRotation = ktm::normalize(finalRotation);
		return finalRotation.matrix4x4();
	}

	/*figures out which scaling keys to interpolate b/w and performs the interpolation
	and returns the scale matrix*/
	ktm::fmat4x4 InterpolateScaling(Bone* Bone, float animationTime)
	{
		if (1 == Bone->m_NumScalings)
			return ktm::scale3d(Bone->m_Scales[0].scale);

		int p0Index = GetScaleIndex(Bone, animationTime);
		int p1Index = p0Index + 1;
		float scaleFactor = GetScaleFactor(Bone->m_Scales[p0Index].timeStamp,
			Bone->m_Scales[p1Index].timeStamp, animationTime);
		ktm::fvec3 finalScale = ktm::lerp(Bone->m_Scales[p0Index].scale, Bone->m_Scales[p1Index].scale
			, scaleFactor);
		return ktm::scale3d(finalScale);
	}


	ktm::fmat4x4 UpdateBone(Bone* Bone, float animationTime)
	{
		ktm::fmat4x4 translation = InterpolatePosition(Bone, animationTime);
		ktm::fmat4x4 rotation = InterpolateRotation(Bone, animationTime);
		ktm::fmat4x4 scale = InterpolateScaling(Bone, animationTime);
		return translation * rotation * scale;
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_CurrentAnimation->m_Bones.begin(), m_CurrentAnimation->m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.m_Name == name;
			}
		);
		if (iter == m_CurrentAnimation->m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	void CalculateBoneTransform(const Animation::AssimpNodeData* node, ktm::fmat4x4 parentTransform)
	{
		std::string nodeName = node->name;
		ktm::fmat4x4 nodeTransform = node->transformation;

		Bone* Bone = FindBone(nodeName);

		if (Bone)
		{
			nodeTransform = UpdateBone(Bone, m_CurrentTime);
		}

		ktm::fmat4x4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->boneInfoMap;
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			ktm::fmat4x4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

private:
	std::vector<ktm::fmat4x4> m_FinalBoneMatrices;

	float m_CurrentTime;
	float m_DeltaTime;

	Animation* m_CurrentAnimation;
	//const Model& model;
};