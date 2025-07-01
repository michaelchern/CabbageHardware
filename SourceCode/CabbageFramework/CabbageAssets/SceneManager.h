#pragma once

#include"CabbageFramework/CabbageFoundation/CabbageFramework.h"
#include"AssetsManager.h"
//#include"ModelImporter.h"
#include "CabbageFramework/CabbageAssets/ModelImporter.h"
#include"CabbageFramework/CabbageMultimedia/CabbageAnimation/CabbageAnimator.h"

struct SceneManager
{
	struct MeshGeomData
	{
		HardwareBuffer pointsBuffer;
		HardwareBuffer normalsBuffer;
		HardwareBuffer texCoordsBuffer;
		HardwareBuffer indexBuffer;
		HardwareBuffer boneIndexesBuffer;
		HardwareBuffer boneWeightsBuffer;

		uint32_t materialIndex;
		uint32_t textureIndex;

		Mesh* meshData;
	};

	struct ActorComponents
	{
		ktm::fmat4x4 modelMatrix = ktm::fmat4x4::from_diag(ktm::fvec4(0.0, 0.0, 0.0, 0.0));
		//ktm::fmat4x4 preModelMatrix = ktm::fmat4x4::from_diag(ktm::fvec4(0.0, 0.0, 0.0, 0.0));
		//ktm::fmat4x4 modelPose = ktm::fmat4x4::from_eye();
        ktm::fvec3 modelTranslate = ktm::fvec3(0.0f, 0.0f, 0.0f);
        ktm::fvec3 modelRoate = ktm::fvec3(0.0f, 0.0f, 0.0f);
		ktm::fvec3 modelScale = ktm::fvec3(1.0f, 1.0f, 1.0f);

		std::unordered_map<std::string, HardwareImage> textureImageHash;

		std::vector<MeshGeomData> geomMeshes;


		HardwareBuffer bonesMatrixBuffer;

		CabbageAnimator animator;
		Model* model;
	};



	void updateScene(EngineOperateList cmdList)
	{
		for (auto item : cmdList.destoryActorList)
		{
			auto iterator = sceneActors.find(item);
			if (iterator != sceneActors.end())
			{
				sceneActors.erase(iterator);
			}
		}

		for (auto item : cmdList.createActorList)
		{

			if (modelPathHash.find(item.second.assetPath) == modelPathHash.end())
			{
				Model tempModel(item.second.assetPath);

				modelPathHash.insert(std::pair<std::string, Model>(item.second.assetPath, tempModel));
			}

			if (sceneActors.find(item.first) == sceneActors.end())
			{
				ActorComponents tempActorComponents;

				for (auto texture : modelPathHash[item.second.assetPath].texturePathHash)
				{
					HardwareImage tempImage = createTextureImage(texture.second);
					tempActorComponents.textureImageHash.insert(std::pair<std::string, HardwareImage>(texture.first, tempImage));
				}

				for (size_t index = 0; index < modelPathHash[item.second.assetPath].meshes.size(); index++)
				{
					MeshGeomData tempMeshData = createMeshGeomData(modelPathHash[item.second.assetPath].meshes[index]);
					tempMeshData.meshData = &modelPathHash[item.second.assetPath].meshes[index];
					tempMeshData.materialIndex = 0;
					tempMeshData.textureIndex = tempActorComponents.textureImageHash[modelPathHash[item.second.assetPath].meshes[index].textures[0].path].storeDescriptor();

					tempActorComponents.geomMeshes.push_back(tempMeshData);
				}

				tempActorComponents.model = &(modelPathHash[item.second.assetPath]);
				//tempActorComponents.animator = CabbageAnimator(&modelPathHash[item.second.assetPath].danceAnimation);

				ktm::fvec3 modelSize = tempActorComponents.model->maxXYZ - tempActorComponents.model->minXYZ;
                //double modelScaleSize = 1.0 / ktm::max(modelSize.x, ktm::max(modelSize.y, modelSize.z));
                double modelScaleSize = 1.0;
                tempActorComponents.modelScale = ktm::fvec3(modelScaleSize, modelScaleSize, modelScaleSize);

				tempActorComponents.modelMatrix = ktm::scale3d(tempActorComponents.modelScale);
				
				sceneActors.insert(std::pair<uint64_t, ActorComponents>(item.first, tempActorComponents));
			}
		}

		for (auto item : cmdList.actorPoseList)
		{
			auto iterator = sceneActors.find(item.first);
			if (iterator != sceneActors.end())
			{
                iterator->second.modelTranslate += cmdList.actorPoseList[item.first].modelTranslate;
                iterator->second.modelRoate += cmdList.actorPoseList[item.first].modelRoate;
				iterator->second.modelScale *= cmdList.actorPoseList[item.first].modelScale;

				ktm::fmat4x4 modelRoate = ktm::rotate3d_axis(ktm::radians(iterator->second.modelRoate.x), ktm::fvec3(1.0, 0.0, 0.0));
                modelRoate *= ktm::rotate3d_axis(ktm::radians(iterator->second.modelRoate.y), ktm::fvec3(0.0, 1.0, 0.0));
                modelRoate *= ktm::rotate3d_axis(ktm::radians(iterator->second.modelRoate.z), ktm::fvec3(0.0, 0.0, 1.0));

                iterator->second.modelMatrix = ktm::translate3d(iterator->second.modelTranslate) * modelRoate * ktm::scale3d(iterator->second.modelScale);
			}
		}


		for (auto& item : sceneActors)
		{
			std::vector<ktm::fmat4x4> transforms = item.second.animator.UpdateBoneAnimation(&(item.second.model->skeletalAnimations[0]), 0.005f);

			if (!item.second.bonesMatrixBuffer)
			{
				item.second.bonesMatrixBuffer = HardwareBuffer(transforms, BufferUsage::StorageBuffer);
			}
			else
			{
				item.second.bonesMatrixBuffer.copyFromData(transforms.data(), transforms.size() * sizeof(ktm::fmat4x4));
			}
		}

	}




	HardwareImage createTextureImage(Texture image)
	{
		return HardwareImage(ktm::uvec2(image.width, image.height), ImageFormat::RGBA8_SRGB, ImageUsage::SampledImage, 1, image.data);

	}


	MeshGeomData createMeshGeomData(Mesh geomMesh)
	{
		MeshGeomData resultMesh;
		resultMesh.pointsBuffer = HardwareBuffer(geomMesh.points, BufferUsage::VertexBuffer);
		resultMesh.normalsBuffer = HardwareBuffer(geomMesh.normals, BufferUsage::VertexBuffer);
		resultMesh.texCoordsBuffer = HardwareBuffer(geomMesh.texCoords, BufferUsage::VertexBuffer);
		resultMesh.boneIndexesBuffer = HardwareBuffer(geomMesh.boneIndices, BufferUsage::VertexBuffer);
		resultMesh.boneWeightsBuffer = HardwareBuffer(geomMesh.boneWeights, BufferUsage::VertexBuffer);
		resultMesh.indexBuffer = HardwareBuffer(geomMesh.triangulatedIndices, BufferUsage::IndexBuffer);
		
		return resultMesh;
	}

	
	std::unordered_map<std::string, Model> modelPathHash;
	std::unordered_map<uint64_t, ActorComponents> sceneActors;

};