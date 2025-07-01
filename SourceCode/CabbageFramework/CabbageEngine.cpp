#pragma once

#include "CabbageEngine.h"

EngineOperateList CabbageEngine::pythonOperateList;
std::vector<CabbageEngine::Scene*> CabbageEngine::sceneList;

CabbageEngine::Scene::Scene(void *surface, bool lightField)
{
    this->surface = surface;
    CabbageEngine::sceneList.push_back(this);
}

CabbageEngine::Scene::~Scene()
{
}

void CabbageEngine::Scene::setCamera(ktm::fvec3 position, ktm::fvec3 forward, ktm::fvec3 worldup, float fov)
{
    pythonOperateList.sceneCamera.cameraPosition = position;
    pythonOperateList.sceneCamera.cameraForward = forward;
    pythonOperateList.sceneCamera.cameraWorldUp = worldup;
    pythonOperateList.sceneCamera.cameraFov = fov;
}

void CabbageEngine::Scene::setSunDirection(ktm::fvec3 dir)
{
    pythonOperateList.sunDir = dir;
}

void CabbageEngine::Scene::setDisplaySurface(void *surface)
{
    this->surface = surface;
}

CabbageEngine::Actor::Actor(Scene &scene, std::string path)
    : actorID(pythonOperateList.createActor(EngineOperateList::ActorBase{std::string(path)}))
{
    scene.AcotrList.push_back(this);
}

CabbageEngine::Actor::~Actor()
{
    pythonOperateList.destoryActor(actorID);
}

void CabbageEngine::Actor::move(ktm::fvec3 pos)
{
    auto iterator = CabbageEngine::pythonOperateList.actorPoseList.find(actorID);
    if (iterator != CabbageEngine::pythonOperateList.actorPoseList.end())
    {
        iterator->second.modelTranslate += pos;
    }
    else
    {
        EngineOperateList::ActorPose tempPose;
        tempPose.modelTranslate = pos;
        CabbageEngine::pythonOperateList.actorPoseList[actorID] = tempPose;
    }
}

void CabbageEngine::Actor::rotate(ktm::fvec3 euler)
{
    auto iterator = CabbageEngine::pythonOperateList.actorPoseList.find(actorID);
    if (iterator != CabbageEngine::pythonOperateList.actorPoseList.end())
    {
        iterator->second.modelRoate += euler;
    }
    else
    {
        EngineOperateList::ActorPose tempPose;
        tempPose.modelRoate = euler;
        CabbageEngine::pythonOperateList.actorPoseList[actorID] = tempPose;
    }
}

void CabbageEngine::Actor::scale(ktm::fvec3 size)
{
    auto iterator = CabbageEngine::pythonOperateList.actorPoseList.find(actorID);
    if (iterator != CabbageEngine::pythonOperateList.actorPoseList.end())
    {
        iterator->second.modelScale += size;
    }
    else
    {
        EngineOperateList::ActorPose tempPose;
        tempPose.modelScale = size;
        CabbageEngine::pythonOperateList.actorPoseList[actorID] = tempPose;
    }
}
