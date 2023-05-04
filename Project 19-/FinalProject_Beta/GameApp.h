#ifndef GAMEAPP_H
#define GAMEAPP_H

#include <random>
#include <WinMin.h>
#include "d3dApp.h"
#include "Effects.h"
#include <CameraController.h>
#include <RenderStates.h>
#include <GameObject.h>
#include <Texture2D.h>
#include <Buffer.h>
#include <Collision.h>
#include <ModelManager.h>
#include <TextureManager.h>


class GameApp : public D3DApp
{

    enum class CameraMode { FirstPerson, ThirdPerson};
    

public:
    GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight);
    ~GameApp();

    bool Init();
    void OnResize();
    void UpdateScene(float dt);
    void DrawScene();

    struct CBChangesEveryFrame
    {
        DirectX::XMMATRIX view;
        DirectX::XMFLOAT4 eyePos;
    };

private:
    bool InitResource();
    
private:


    TextureManager m_TextureManager;
    ModelManager m_ModelManager;

    BasicEffect m_BasicEffect;		            			    // 对象渲染特效管理
    SkyboxEffect m_SkyboxEffect;							    // 天空盒特效管理
    bool m_EnableNormalMap;                                     // 法线贴图开关
    std::unique_ptr<Depth2D> m_pDepthTexture;                   // 深度缓冲区

    GameObject m_Skybox;                                        // 天空盒
    GameObject m_Ground;										// 地面
    GameObject m_GroundSide[2];                                 // 道路周围
    GameObject m_LeftBarriers[20];                              // 左围栏
    GameObject m_RightBarriers[20];                             // 右围栏
    GameObject m_Car;                                           // 小车
    GameObject m_LeftTree[15];                                  // 左边的树
    GameObject m_RightTree[15];                                 // 右边的树

    CameraMode m_CameraMode;                                    // 摄像机状态
    
    int i;
    int j;
   
    std::shared_ptr<FirstPersonCamera> m_pCamera;			    // 玩家摄像机
    FirstPersonCameraController m_CameraController;             // 玩家摄像机控制器 
    


};

#endif


