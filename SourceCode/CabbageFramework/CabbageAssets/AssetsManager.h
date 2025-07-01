#pragma once

#include <string>
#include <set>

#include <ktm/ktm.h>


class EngineOperateList
{
public:
    struct CameraManager
    {
        float cameraFov = 45.0f;
        ktm::fvec3 cameraPosition = ktm::fvec3(1.0f, 1.0f, 1.0f);
        ktm::fvec3 cameraForward = ktm::fvec3(-1.0f, -1.0f, -1.0f);
        ktm::fvec3 cameraWorldUp = ktm::fvec3(0.0f, 1.0f, 0.0f);
    };

    struct ActorPose
    {
        ktm::fvec3 modelTranslate = ktm::fvec3(0.0f, 0.0f, 0.0f);
        ktm::fvec3 modelRoate = ktm::fvec3(0.0f, 0.0f, 0.0f);
        ktm::fvec3 modelScale = ktm::fvec3(1.0f, 1.0f, 1.0f);
    };

    struct ActorBase
    {
        std::string assetPath;
        // std::string physicalState;
        // ktm::fvec3 location = ktm::fvec3(1.0f, 1.0f, 1.0f);
    };

	uint64_t createActor(ActorBase base)
	{
		createActorList[++actorCurrentIndex] = base;
		return actorCurrentIndex;
	}

	void destoryActor(uint64_t actorIndex)
	{
		auto iterator = createActorList.find(actorIndex);
		if (iterator == createActorList.end())
		{
			destoryActorList.insert(actorIndex);
		}
		else
		{
			createActorList.erase(iterator);
		}
	}

	void changeActorPose(int actorIndex, ActorPose actorPose)
	{
		if (actorPoseList.find(actorIndex) == actorPoseList.end())
		{
			actorPoseList[actorIndex] = actorPose;
		}
		else
		{
			actorPoseList[actorIndex].modelTranslate += actorPose.modelTranslate;
            actorPoseList[actorIndex].modelRoate += actorPose.modelRoate;
			actorPoseList[actorIndex].modelScale *= actorPose.modelScale;
		}
	}


	void clearList()
	{
		createActorList.clear();
		destoryActorList.clear();
		actorPoseList.clear();
	}

	void mergeOperate(EngineOperateList& origin)
	{
        sunDir = origin.sunDir;

		//displaySize = origin.displaySize;
		//surface = origin.surface;

		sceneCamera = origin.sceneCamera;

		actorCurrentIndex = origin.actorCurrentIndex;

		for (auto item : origin.createActorList)
		{
			createActorList[item.first] = item.second;
		}

		for (auto item : origin.destoryActorList)
		{
			destoryActor(item);
		}

		for (auto item : origin.actorPoseList)
		{
			changeActorPose((int)item.first, item.second);
		}

		origin.clearList();
		origin.physxActorList = physxActorList;
		origin.actorsMatrix = actorsMatrix;
		origin.collisionActors = collisionActors;
	}


	std::unordered_map<uint64_t, ActorBase> createActorList;
	std::set<uint64_t> destoryActorList;
	std::unordered_map<uint64_t, ActorPose> actorPoseList;

	std::unordered_map<uint64_t, ktm::fvec3> physxActorList;
	std::unordered_map<uint64_t, std::set<uint64_t>> collisionActors;

	std::unordered_map<uint64_t, ktm::fmat4x4> actorsMatrix;

	CameraManager sceneCamera;

	//ktm::uvec2 displaySize = ktm::uvec2(0,0);
	//void* surface = nullptr;

	ktm::fvec3 sunDir = ktm::fvec3(0.0,1.0,0.0);

private:

	uint64_t actorCurrentIndex = 0;

	friend EngineOperateList;
};