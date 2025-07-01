#pragma once

#include <string>

#include <ktm/ktm.h>


#include "CabbageFramework/CabbageAssets/AssetsManager.h"



struct CabbageEngine
{
    struct Actor;
    struct Scene;

    CabbageEngine() = delete;
    ~CabbageEngine() = delete;

    struct Scene
    {
      public:
        Scene(void *surface = nullptr, bool lightField = false);
        ~Scene();

        void setCamera(ktm::fvec3 position, ktm::fvec3 forward, ktm::fvec3 worldup, float fov);
        void setSunDirection(ktm::fvec3 dir);
        void setDisplaySurface(void *surface);

        void detectActorIndexByRay(ktm::fvec3 origin, ktm::fvec3 dir) {};
        void detectActorIndexByScreen(ktm::uvec2 pixel) {};

      //private:
        const uint64_t sceneID = std::numeric_limits<uint64_t>::max();
        void* surface = nullptr;
        std::vector<Actor*> AcotrList;
    };

    
    struct Actor
    {
      public:
        Actor(Scene &scene, std::string path = "");
        ~Actor();

        void move(ktm::fvec3 pos);
        void rotate(ktm::fvec3 euler);
        void scale(ktm::fvec3 size);

        void setMeshShape(std::string path) {};

        void detectCollision() {};

        struct OpticsParams
        {
            bool enable;
        };
        void setOpticsParams(const OpticsParams &params) {};

        struct AcousticsParams
        {
            bool enable;
        };
        void setAcousticsParams(const AcousticsParams &params) {};

        struct MechanicsParams
        {
            bool enable;
        };
        void setMechanicsParams(const MechanicsParams &params) {};

      //private:
        const uint64_t actorID = std::numeric_limits<uint64_t>::max();
    };

    static EngineOperateList pythonOperateList;
    static std::vector<Scene*> sceneList;
};