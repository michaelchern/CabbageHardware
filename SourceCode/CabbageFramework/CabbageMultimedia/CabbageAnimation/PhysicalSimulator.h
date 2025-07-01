#pragma once

#include "CabbageEditor/PythonAPI.h"

#include"CabbageFramework/CabbageAssets/SceneManager.h"

#include"iostream"

#include"vector"

#include"unordered_map"

#include"CabbageFramework/CabbageAssets/ModelImporter.h"


#include <CabbageEngine.h>


#include"utility"



struct PhysicalSimulator
{
    PhysicalSimulator(SceneManager &sceneManager)
        : sceneManager(sceneManager)
    {
    }
    std::unordered_map<int, std::set<uint64_t>> collisionActors;           //һ�Զ�Ĺ�ϵ����ײ��
    // ���ڶ������ײ��⣺���������Χ�еĶ����Ƿ����ڶԷ����ڵ����
    bool checkCollision(const std::vector<ktm::fvec3> &vertices1p, const std::vector<ktm::fvec3> vertices2p)
    {
        // ����vertices2��AABB�߽�
        ktm::fvec3 min2 = vertices2p[0];
        ktm::fvec3 max2 = vertices2p[0];
        for (const auto &v : vertices2p)
        {
            min2 = ktm::min(min2, v);
            max2 = ktm::max(max2, v);
        }

        // ���vertices1�����ж����Ƿ���vertices2��AABB��
        for (const auto &point : vertices1p)
        {
            if (point.x >= min2.x && point.x <= max2.x &&
                point.y >= min2.y && point.y <= max2.y &&
                point.z >= min2.z && point.z <= max2.z)
            {
                return true;
            }
        }

        // ����vertices1��AABB�߽�
        ktm::fvec3 min1 = vertices1p[0];
        ktm::fvec3 max1 = vertices1p[0];
        for (const auto &v : vertices1p)
        {
            min1 = ktm::min(min1, v);
            max1 = ktm::max(max1, v);
        }

        // ���vertices2�����ж����Ƿ���vertices1��AABB��
        for (const auto &point : vertices2p)
        {
            if (point.x >= min1.x && point.x <= max1.x &&
                point.y >= min1.y && point.y <= max1.y &&
                point.z >= min1.z && point.z <= max1.z)
            {
                return true;
            }
        }

        return false;
    }
  
 
    std::vector<ktm::fvec3> calculateVertices(const ktm::fvec3 &startMin, const ktm::fvec3 &startMax)
    {
        std::vector<ktm::fvec3> vertices = {
            ktm::fvec3(startMin.x, startMin.y, startMin.z),
            ktm::fvec3(startMin.x, startMin.y, startMax.z),
            ktm::fvec3(startMin.x, startMax.y, startMin.z),
            ktm::fvec3(startMin.x, startMax.y, startMax.z),
            ktm::fvec3(startMax.x, startMin.y, startMin.z),
            ktm::fvec3(startMax.x, startMin.y, startMax.z),
            ktm::fvec3(startMax.x, startMax.y, startMin.z),
            ktm::fvec3(startMax.x, startMax.y, startMax.z)};
        return std::move(vertices);
    }


    bool update()
    {
        collisionActors.clear();
        for (auto &actor : sceneManager.sceneActors) // ����������������
        {
            uint64_t actorIndex = actor.first;
            auto &actorModelInfo = actor.second.model;
            auto StartMin = actor.second.model->minXYZ;
            auto StartMax = actor.second.model->maxXYZ;
            auto &ChangeMatrix = actor.second.modelMatrix;
            std::vector<ktm::fvec3>Vertices1 = calculateVertices(StartMin, StartMax);
            for (int k = 0; k < 8; ++k) // ת������������
            {
                Vertices1[k] = ktm::fvec4(ChangeMatrix * ktm::fvec4(Vertices1[k], 0.0)).xyz();
            }
            for (auto &otherActor : sceneManager.sceneActors)
            {
                uint64_t otherActorIndex = otherActor.first;
                auto &otheractorModelInfo = otherActor.second.model;
                auto otherStartMin = otherActor.second.model->minXYZ;
                auto otherStartMax = otherActor.second.model->maxXYZ;
                auto &ChangeMatrix = otherActor.second.modelMatrix;
                std::vector<ktm::fvec3> Vertices2 = calculateVertices(otherStartMin, otherStartMax);
                // �����������������
                if (&actor == &otherActor)
                    continue;
                for (int k = 0; k < 8; ++k) // ת������������
                {
                    Vertices2[k] = ktm::fvec4(ChangeMatrix * ktm::fvec4(Vertices2[k], 0.0)).xyz();
                }
                bool collisionResult = checkCollision(Vertices1, Vertices2);
                if (collisionResult)
                {
                    // ��¼��ײ��ϵ��˫��
                    collisionActors[actorIndex].insert(otherActorIndex);
                    collisionActors[otherActorIndex].insert(actorIndex);
                }
            }
        }
        return true;
    }
    SceneManager &sceneManager;
};