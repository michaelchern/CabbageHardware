#include <iostream>

//#include "CabbageEditor/PythonAPI.h"

#include "CabbageFramework/CabbageGlobalContext.h"
#include "CabbageFramework/CabbageAssets/SceneManager.h"

#include<thread>

#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


int main()
{
    //auto lammdaReflect = [&](std::string_view name, auto &&structMember, std::size_t i) {
    //    std::cout << "Member index: " << i
    //              << ", Member Value Type: " << typeid(structMember).name()
    //              << ", Member Name: " << name
    //              << std::endl;
    //};
    //boost::pfr::for_each_field_with_name(CabbageEngine{}, lammdaReflect);

    CabbageEngine::Scene tempscene;
    //CabbageEngine::Actor temp = CabbageEngine::Actor(tempscene, "E:\\CabbageEngine\\TestCase\\vampire\\dancing_vampire.dae");

    //PythonAPI pythonManager;
    //std::thread([&]() {
    //    while (true)
    //    {
    //        pythonManager.checkPythonScriptChange();
    //        pythonManager.checkReleaseScriptChange();
    //        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    //    }
    //}).detach();

    //EngineOperateList engineCmdList;

    //std::mutex mtx;

    //std::thread([&]() {
    //    while (true)
    //    {
    //        pythonManager.runPythonScript();

    //        mtx.lock();
    //        engineCmdList.mergeOperate(CabbageEngine::pythonOperateList);
    //        mtx.unlock();
    //    }
    //}).detach();

    std::thread([&]() {
        if (glfwInit() >= 0)
        {
            GLFWwindow *window;
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            window = glfwCreateWindow(800, 800, "Cabbage Engine", nullptr, nullptr);
            void *surface = glfwGetWin32Window(window);

            tempscene.setDisplaySurface(surface);

            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
            }

            glfwDestroyWindow(window);

            glfwTerminate();
        }
    }).detach();

    CabbageGlobalContext app;
    EngineOperateList originCmdList;
    while (true)
    {
        //mtx.lock();
        //EngineOperateList originCmdList = engineCmdList;
        //engineCmdList.clearList();
        //mtx.unlock();

        app.updateEngine(originCmdList);
    }

    return 0;
}