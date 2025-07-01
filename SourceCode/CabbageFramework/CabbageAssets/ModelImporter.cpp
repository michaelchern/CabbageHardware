
#include"ModelImporter.h"

#include <regex>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


Model::Model(std::string path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	processNode(path, scene->mRootNode, scene);

	skeletalAnimations.resize(scene->mNumAnimations);
	for (size_t i = 0; i < scene->mNumAnimations; i++)
	{
		skeletalAnimations[i] = Animation(scene, scene->mAnimations[i], m_BoneInfoMap, m_BoneCounter);
	}

	if (meshes.size()>0)
    {
        minXYZ = meshes[0].minXYZ;
        maxXYZ = meshes[0].maxXYZ;
    }
	for (size_t i = 0; i < meshes.size(); i++)
    {
        maxXYZ = ktm::fvec3(ktm::max(meshes[i].maxXYZ.x, maxXYZ.x),
                            ktm::max(meshes[i].maxXYZ.y, maxXYZ.y),
                            ktm::max(meshes[i].maxXYZ.z, maxXYZ.z));

        minXYZ = ktm::fvec3(ktm::min(meshes[i].minXYZ.x, minXYZ.x),
                            ktm::min(meshes[i].minXYZ.y, minXYZ.y),
                            ktm::min(meshes[i].minXYZ.z, minXYZ.z));
    }
}


void Model::processNode(std::string path, aiNode* node, const aiScene* scene)
{
	// �����ڵ����е���������еĻ���
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(path, mesh, scene));
	}
	// �������������ӽڵ��ظ���һ����
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(path, node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(std::string path, aiMesh* mesh, const aiScene* scene)
{
	Mesh resultMesh;

	if (mesh->mNumVertices > 0)
	{
		resultMesh.minXYZ = resultMesh.maxXYZ = ktm::fvec3(mesh->mVertices[0].x, mesh->mVertices[0].y, mesh->mVertices[0].z);
	}

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		resultMesh.points.push_back(mesh->mVertices[i].x);
		resultMesh.points.push_back(mesh->mVertices[i].y);
		resultMesh.points.push_back(mesh->mVertices[i].z);


		resultMesh.maxXYZ = ktm::fvec3(ktm::max(mesh->mVertices[i].x, resultMesh.maxXYZ.x),
                                       ktm::max(mesh->mVertices[i].y, resultMesh.maxXYZ.y),
                                       ktm::max(mesh->mVertices[i].z, resultMesh.maxXYZ.z));

        resultMesh.minXYZ = ktm::fvec3(ktm::min(mesh->mVertices[i].x, resultMesh.minXYZ.x),
                                       ktm::min(mesh->mVertices[i].y, resultMesh.minXYZ.y),
                                       ktm::min(mesh->mVertices[i].z, resultMesh.minXYZ.z));


		if (mesh->HasNormals())
		{
			resultMesh.normals.push_back(mesh->mNormals[i].x);
			resultMesh.normals.push_back(mesh->mNormals[i].y);
			resultMesh.normals.push_back(mesh->mNormals[i].z);
		}

		if (mesh->HasTextureCoords(0))
		{
			resultMesh.texCoords.push_back(mesh->mTextureCoords[0][i].x);
			resultMesh.texCoords.push_back(mesh->mTextureCoords[0][i].y);
		}
		else
		{
			resultMesh.texCoords.push_back(0.0f);
			resultMesh.texCoords.push_back(0.0f);
		}

		resultMesh.boneIndices.push_back(0);
		resultMesh.boneIndices.push_back(0);
		resultMesh.boneIndices.push_back(0);
		resultMesh.boneIndices.push_back(0);

		resultMesh.boneWeights.push_back(0.0f);
		resultMesh.boneWeights.push_back(0.0f);
		resultMesh.boneWeights.push_back(0.0f);
		resultMesh.boneWeights.push_back(0.0f);
	}

	ExtractBoneWeightForVertices(resultMesh, mesh, scene);

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			resultMesh.triangulatedIndices.push_back(face.mIndices[j]);
		}
	}


	if (mesh->mMaterialIndex >= 0)
	{
		loadMaterial(path, scene->mMaterials[mesh->mMaterialIndex], resultMesh);
	}

	return resultMesh;
}

void Model::ExtractBoneWeightForVertices(Mesh& resultMesh, aiMesh* mesh, const aiScene* scene)
{
	auto& boneInfoMap = m_BoneInfoMap;
	int& boneCount = m_BoneCounter;

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

	for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = boneCount;
			newBoneInfo.offset = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
			boneInfoMap[boneName] = newBoneInfo;
			boneID = boneCount;
			boneCount++;
		}
		else
		{
			boneID = boneInfoMap[boneName].id;
		}
		//assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;

			//SetVertexBoneData(resultMesh.[vertexId], boneID, weight);
			for (int i = 0; i < 4; ++i)
			{
				if ((resultMesh.boneIndices[vertexId * 4 + i] == 0) && (resultMesh.boneWeights[vertexId * 4 + i] <= 1e-3))
				{
					resultMesh.boneWeights[vertexId * 4 + i] = weight;
					resultMesh.boneIndices[vertexId * 4 + i] = boneID;
					break;
				}
			}
		}
	}
}

void Model::loadMaterial(std::string path, aiMaterial* material, Mesh& resultMesh)
{
	std::string meshRootPath = "";
	std::regex rexPattern("(.*)((\\\\)|(/))");
	std::smatch matchPath;
	if (regex_search(path, matchPath, rexPattern))
	{
		meshRootPath = matchPath[1].str();
	}
	else
	{
		throw "mesh path invalid in python";
	}
	std::string directory = meshRootPath;


	std::vector<aiTextureType> allTextureTypes = {
		aiTextureType_BASE_COLOR,
		aiTextureType_DIFFUSE,
		aiTextureType_SPECULAR,
		aiTextureType_EMISSIVE,
		aiTextureType_EMISSION_COLOR
	};

	for (size_t index = 0; index < allTextureTypes.size(); index++)
	{
		std::vector<Texture> textures;
		for (unsigned int i = 0; i < material->GetTextureCount(allTextureTypes[index]); i++)
		{
			aiString str;
			material->GetTexture(allTextureTypes[index], i, &str);

			Texture texture;
			texture.type = allTextureTypes[index];
			texture.path = directory + "\\" + str.C_Str();

			if (texturePathHash.find(texture.path) == texturePathHash.end())
			{
				texture.data = stbi_load(texture.path.c_str(), &texture.width, &texture.height, &texture.nrChannels, 0);
				texturePathHash.insert(std::pair<std::string, Texture>(texture.path, texture));
			}

			textures.push_back(texture);
		}

		resultMesh.textures.insert(resultMesh.textures.end(), textures.begin(), textures.end());
	}

	aiColor3D baseColor(0.f, 0.f, 0.f);
	if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == aiReturn_SUCCESS)
	{
		Texture tempTexture;
		tempTexture.path = directory + std::to_string(++attributeToImageIndex);
		tempTexture.type = aiTextureType_BASE_COLOR;

		if (texturePathHash.find(tempTexture.path) == texturePathHash.end())
		{
			tempTexture.width = 1;
			tempTexture.height = 1;
			tempTexture.nrChannels = 4;
			tempTexture.data = (unsigned char*)malloc(sizeof(unsigned char) * 4);
			tempTexture.data[0] = (unsigned char)(baseColor[0] * 255.0);
			tempTexture.data[1] = (unsigned char)(baseColor[1] * 255.0);
			tempTexture.data[2] = (unsigned char)(baseColor[2] * 255.0);
			tempTexture.data[3] = 255;

			resultMesh.textures.push_back(tempTexture);

			texturePathHash.insert(std::pair<std::string, Texture>(tempTexture.path, tempTexture));
		}
	}

	aiColor3D specColor(0.f, 0.f, 0.f);
	if (material->Get(AI_MATKEY_COLOR_SPECULAR, specColor) == aiReturn_SUCCESS)
	{
		Texture tempTexture;
		tempTexture.path = directory + std::to_string(++attributeToImageIndex);
		tempTexture.type = aiTextureType_SPECULAR;

		if (texturePathHash.find(tempTexture.path) == texturePathHash.end())
		{
			tempTexture.width = 1;
			tempTexture.height = 1;
			tempTexture.nrChannels = 4;
			tempTexture.data = (unsigned char*)malloc(sizeof(unsigned char) * 4);
			tempTexture.data[0] = (unsigned char)(specColor[0] * 255.0);
			tempTexture.data[1] = (unsigned char)(specColor[1] * 255.0);
			tempTexture.data[2] = (unsigned char)(specColor[2] * 255.0);
			tempTexture.data[3] = 255;

			resultMesh.textures.push_back(tempTexture);
			texturePathHash.insert(std::pair<std::string, Texture>(tempTexture.path, tempTexture));
		}
	}

	aiColor3D diffuseColor(0.f, 0.f, 0.f);
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn_SUCCESS)
	{
		Texture tempTexture;
		tempTexture.path = directory + std::to_string(++attributeToImageIndex);
		tempTexture.type = aiTextureType_DIFFUSE;

		if (texturePathHash.find(tempTexture.path) == texturePathHash.end())
		{
			tempTexture.width = 1;
			tempTexture.height = 1;
			tempTexture.nrChannels = 4;
			tempTexture.data = (unsigned char*)malloc(sizeof(unsigned char) * 4);
			tempTexture.data[0] = (unsigned char)(diffuseColor[0] * 255.0);
			tempTexture.data[1] = (unsigned char)(diffuseColor[1] * 255.0);
			tempTexture.data[2] = (unsigned char)(diffuseColor[2] * 255.0);
			tempTexture.data[3] = 255;

			resultMesh.textures.push_back(tempTexture);
			texturePathHash.insert(std::pair<std::string, Texture>(tempTexture.path, tempTexture));
		}
	}

	aiColor3D emissiveColor(0.f, 0.f, 0.f);
	if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor) == aiReturn_SUCCESS)
	{
		Texture tempTexture;
		tempTexture.path = directory + std::to_string(++attributeToImageIndex);
		tempTexture.type = aiTextureType_EMISSIVE;

		if (texturePathHash.find(tempTexture.path) == texturePathHash.end())
		{
			tempTexture.width = 1;
			tempTexture.height = 1;
			tempTexture.nrChannels = 4;
			tempTexture.data = (unsigned char*)malloc(sizeof(unsigned char) * 4);
			tempTexture.data[0] = (unsigned char)(emissiveColor[0] * 255.0);
			tempTexture.data[1] = (unsigned char)(emissiveColor[1] * 255.0);
			tempTexture.data[2] = (unsigned char)(emissiveColor[2] * 255.0);
			tempTexture.data[3] = 255;

			resultMesh.textures.push_back(tempTexture);
			texturePathHash.insert(std::pair<std::string, Texture>(tempTexture.path, tempTexture));
		}
	}

	float Metallic = 0.0f;
	material->Get(AI_MATKEY_METALLIC_FACTOR, Metallic);

	float Roughness = 0.0f;
	material->Get(AI_MATKEY_ROUGHNESS_FACTOR, Roughness);

	float Specular = 0.0f;
	material->Get(AI_MATKEY_SPECULAR_FACTOR, Specular);

	float Glossiness = 0.0;
	material->Get(AI_MATKEY_GLOSSINESS_FACTOR, Glossiness);
}

