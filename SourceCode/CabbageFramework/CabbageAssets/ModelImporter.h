#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>

#include <ktm/ktm.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "CabbageFramework/CabbageFoundation/CabbageFramework.h"


struct BoneInfo
{
	int id;  //id is index in finalBoneMatrices
	ktm::fmat4x4 offset;  //offset matrix transforms vertex from model space to bone space
};

struct Texture
{
	aiTextureType type;
	std::string path;
	unsigned char* data;
	int width, height, nrChannels;
};

struct Mesh
{
	ktm::fvec3 minXYZ = ktm::fvec3(0.0f, 0.0f, 0.0f);
	ktm::fvec3 maxXYZ = ktm::fvec3(0.0f, 0.0f, 0.0f);

	std::vector<uint32_t> triangulatedIndices;              // Triangle point indices

	std::vector<float> points;                             // Vertex positions (always indexed)
	std::vector<float> normals;                            // Normals; indexed iff normalInterp is vertex or varying
	std::vector<float> texCoords;                            // Texture coordinates, indexed iff texCrdsInterp is vertex or varying
	std::vector<uint32_t> boneIndices;                        // Bone indices
	std::vector<float> boneWeights;                       // Bone weights corresponding to the bones referenced in jointIndices

	std::vector<Texture> textures;
};


struct Bone
{
	struct KeyPosition
	{
		ktm::fvec3 position;
		float timeStamp;
	};

	struct KeyRotation
	{
		ktm::fquat orientation;
		float timeStamp;
	};

	struct KeyScale
	{
		ktm::fvec3 scale;
		float timeStamp;
	};

	std::vector<KeyPosition> m_Positions;
	std::vector<KeyRotation> m_Rotations;
	std::vector<KeyScale> m_Scales;

	int m_NumPositions;
	int m_NumRotations;
	int m_NumScalings;

	std::string m_Name;
	int m_ID;

	Bone(const std::string& name, int ID, const aiNodeAnim* channel) :m_Name(name), m_ID(ID)
	{
		m_NumPositions = channel->mNumPositionKeys;

		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
		{
			aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
			double timeStamp = channel->mPositionKeys[positionIndex].mTime;
			KeyPosition data;
			data.position = ktm::fvec3(aiPosition[0], aiPosition[1], aiPosition[2]);
			data.timeStamp = (float)timeStamp;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;
		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
		{
			aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
			double timeStamp = channel->mRotationKeys[rotationIndex].mTime;
			KeyRotation data;
			data.orientation = ktm::fquat(aiOrientation.x, aiOrientation.y, aiOrientation.z, aiOrientation.w);
			data.timeStamp = (float)timeStamp;
			m_Rotations.push_back(data);
		}

		m_NumScalings = channel->mNumScalingKeys;
		for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
		{
			aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
			double timeStamp = channel->mScalingKeys[keyIndex].mTime;
			KeyScale data;
			data.scale = ktm::fvec3(scale[0], scale[1], scale[2]);
			data.timeStamp = (float)timeStamp;
			m_Scales.push_back(data);
		}
	}
};


struct Animation
{
	struct AssimpNodeData
	{
		ktm::fmat4x4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};

	Animation() = default;
	~Animation() = default;

	Animation(const aiScene* scene, aiAnimation* animation, std::map<std::string, BoneInfo> boneInfoMap, int& boneCount)
		:boneInfoMap(boneInfoMap)
	{
		//Assimp::Importer importer;
		//const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
		////assert(scene && scene->mRootNode);
		//auto animation = scene->mAnimations[0];
		m_Duration = animation->mDuration;
		m_TicksPerSecond = animation->mTicksPerSecond;

		auto ReadMissingBones = [&](const aiAnimation* animation, std::map<std::string, BoneInfo>& boneInfoMap, int& boneCount)
			{
				//reading channels(bones engaged in an animation and their keyframes)
				for (uint32_t i = 0; i < animation->mNumChannels; i++)
				{
					auto channel = animation->mChannels[i];
					std::string boneName = channel->mNodeName.data;

					if (boneInfoMap.find(boneName) == boneInfoMap.end())
					{
						boneInfoMap[boneName].id = boneCount;
						boneCount++;
					}
					m_Bones.push_back(Bone(channel->mNodeName.data,
						boneInfoMap[channel->mNodeName.data].id, channel));
				}
			};

		ReadHeirarchyData(m_RootNode, scene->mRootNode);
		ReadMissingBones(animation, boneInfoMap, boneCount);
	}

	void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
	{
		auto ConvertMatrixToGLMFormat = [](const aiMatrix4x4& from) -> ktm::fmat4x4
			{
				ktm::fmat4x4 to;
				//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
				to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
				to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
				to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
				to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
				return to;
			};

		dest.name = src->mName.data;
		dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
		dest.childrenCount = src->mNumChildren;

		for (uint32_t i = 0; i < src->mNumChildren; i++)
		{
			AssimpNodeData newData;
			ReadHeirarchyData(newData, src->mChildren[i]);
			dest.children.push_back(newData);
		}
	}

	double m_Duration;
	double m_TicksPerSecond;
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> boneInfoMap;
};

struct Model
{
	Model() = default;
	~Model() = default;

	Model(std::string path);

	void processNode(std::string path, aiNode* node, const aiScene* scene);
	Mesh processMesh(std::string path, aiMesh* mesh, const aiScene* scene);
	void ExtractBoneWeightForVertices(Mesh& resultMesh, aiMesh* mesh, const aiScene* scene);
	void loadMaterial(std::string path, aiMaterial* material, Mesh& resultMesh);

	std::vector<Mesh> meshes;

	std::map<std::string, BoneInfo> m_BoneInfoMap;
	int m_BoneCounter = 0;

	std::vector<Animation> skeletalAnimations;

	std::unordered_map<std::string, Texture> texturePathHash;
	uint32_t attributeToImageIndex = 0;

	ktm::fvec3 minXYZ = ktm::fvec3(0.0f, 0.0f, 0.0f);
    ktm::fvec3 maxXYZ = ktm::fvec3(0.0f, 0.0f, 0.0f);
};
