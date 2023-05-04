#include "GameApp.h"
#include <XUtil.h>
#include <DXTrace.h>
#define HeadBufferSize 256
using namespace DirectX;


GameApp::GameApp(HINSTANCE hInstance, const std::wstring& windowName, int initWidth, int initHeight)
    : D3DApp(hInstance, windowName, initWidth, initHeight),
    m_EnableNormalMap(true)
    
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
    if (!D3DApp::Init())
        return false;

    m_TextureManager.Init(m_pd3dDevice.Get());
    m_ModelManager.Init(m_pd3dDevice.Get());

    // 务必先初始化所有渲染状态，以供下面的特效使用
    RenderStates::InitAll(m_pd3dDevice.Get());

    if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!m_SkyboxEffect.InitAll(m_pd3dDevice.Get()))
        return false;

    if (!InitResource())
        return false;

    return true;
}

void GameApp::OnResize()
{

    D3DApp::OnResize();
    
    m_pDepthTexture = std::make_unique<Depth2D>(m_pd3dDevice.Get(), m_ClientWidth, m_ClientHeight);
    m_pDepthTexture->SetDebugObjectName("DepthTexture");

    // 摄像机变更显示
    if (m_pCamera != nullptr)
    {
        m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
        m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
        m_BasicEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
        m_SkyboxEffect.SetProjMatrix(m_pCamera->GetProjMatrixXM());
    }
}

void GameApp::UpdateScene(float dt)
{

    //当前效果：可周期性变化光照强度，但如果direction.z > 1.2f 后除了树以外的物体都会照成白色
    static float phi2 = 0.001f;
    phi2 += 0.01f;
    // 方向光
    DirectionalLight dirLight[4]{};
    dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
    dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f * 2.0f * sin(0.006f * phi2));
    dirLight[1] = dirLight[0];
    dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f * 2.0f * sin(0.006f * phi2));
    dirLight[2] = dirLight[0];
    dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f * 2.0f * sin(0.006f * phi2));
    dirLight[3] = dirLight[0];
    dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f * 2.0f * sin(0.006f * phi2));
    for (int i = 0; i < 4; ++i)
        m_BasicEffect.SetDirLight(i, dirLight[i]);

    m_CameraController.Update(dt);
    ImGuiIO& io = ImGui::GetIO();
    Transform& CarTransform =m_Car.GetTransform();
    //加速模拟，按下w键向前加速，按下s向后加速
    static float v0 = 0.0f;//小车初速度
    if (ImGui::IsKeyDown(ImGuiKey_W))
    {
        v0 += 40.0f * dt;
        m_CameraController.SetMoveSpeed(v0);
    }
    //松开w键后减速
    else if ( v0 > 0.0f)
    {
        v0 -= 10.0f * dt;
        v0 = v0 > 0.0f ? v0 : 0.0f;
        m_pCamera->MoveForward(v0*dt);
    }

    if (ImGui::IsKeyDown(ImGuiKey_S))
    {
        v0 -= 40.0f * dt;
        m_CameraController.SetMoveSpeed(-v0);
    }
    //松开s键后减速
    else if (v0 < 0.0f)
    {
        v0 += 10.0f * dt;
        v0 = v0 < 0.0f ? v0 : 0.0f;
        m_pCamera->MoveForward(v0 * dt);
    }

    //减速模拟，按下空格键减速
    if (ImGui::IsKeyDown(ImGuiKey_Space))
    {
        if (v0 > 0.0f)
        {
            v0 -= 75.0f * dt;
            v0 = v0 > 0.0f ? v0 : 0.0f;
            m_CameraController.SetMoveSpeed(v0);
        }
        else
        {
                v0 += 75.0f * dt;
                v0 = v0 < 0.0f ? v0 : 0.0f;
                m_CameraController.SetMoveSpeed(-v0);   
        }
    }

    //限制摄像机活动范围(第一人称)
    XMFLOAT3 adjustedPos = CarTransform.GetPosition();
    XMStoreFloat3(&adjustedPos, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-8.0f, -2.99f, -70.0f, 0.0f), XMVectorSet(8.0f, -2.99f, 70.0f, 0.0f)));
    //让小车跟随摄像机移动
    if (m_CameraMode == CameraMode::FirstPerson)
    {   
        //限制摄像机活动范围(第一人称)
        /*XMStoreFloat3(&adjustedPos, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-10.0f, -2.99f, CarTransform.GetPosition().z-1.0f, 0.0f), XMVectorSet(10.0f, -2.99f, 100.0f, 0.0f)));
        m_pCamera->SetPosition(adjustedPos);*/
        XMFLOAT3 FirstPersonView;
        FirstPersonView.x = adjustedPos.x;
        FirstPersonView.y = adjustedPos.y+ 2.0f;
        FirstPersonView.z = adjustedPos.z;
        m_pCamera->SetPosition(FirstPersonView);
        XMStoreFloat3(&FirstPersonView, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-8.0f, 2.0f, -80.0f, 0.0f), XMVectorSet(8.0f, 2.0f, 80.0f, 0.0f)));
        //让小车跟随摄像机移动
        XMStoreFloat3(&adjustedPos, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-8.0f, -2.99f, m_pCamera->GetPosition().z -2.0f, 0.0f), XMVectorSet(8.0f, -2.99f, m_pCamera->GetPosition().z - 2.0f, 0.0f)));
        CarTransform.SetPosition(adjustedPos);
    }

    else if (m_CameraMode == CameraMode::ThirdPerson)
    { 
        //限制摄像机活动范围(第三人称)
        XMFLOAT3 ThirdPersonView;
        ThirdPersonView.x = adjustedPos.x;
        ThirdPersonView.y = adjustedPos.y + 3.0f;
        ThirdPersonView.z = adjustedPos.z;
        m_pCamera->SetPosition(ThirdPersonView);
        XMStoreFloat3(&ThirdPersonView, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-10.0f, 2.0f, -80.0f, 0.0f), XMVectorSet(10.0f, 2.0f, 80.0f, 0.0f)));
        m_pCamera->LookTo(ThirdPersonView, XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));//固定相机观察方向
        //使物体位置限制在摄像机前
        XMStoreFloat3(&adjustedPos, XMVectorClamp(m_pCamera->GetPositionXM(), XMVectorSet(-10.0f, -1.0f, m_pCamera->GetPosition().z+10.0f, 0.0f), XMVectorSet(10.0f, -1.0f, m_pCamera->GetPosition().z + 10.0f, 0.0f)));
        CarTransform.SetPosition(adjustedPos);
        
    }

    if (ImGui::IsKeyPressed(ImGuiKey_C) && m_CameraMode != CameraMode::FirstPerson)
    {
        m_CameraMode = CameraMode::FirstPerson;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_C) && m_CameraMode != CameraMode::ThirdPerson)
    {
        m_CameraMode = CameraMode::ThirdPerson;
    }

    // 更新观察矩阵
    m_BasicEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());
    m_BasicEffect.SetEyePos(m_pCamera->GetPosition());
    m_SkyboxEffect.SetViewMatrix(m_pCamera->GetViewMatrixXM());

    if (ImGui::Begin("Static Cube Mapping"))
    {
        ImGui::Checkbox("Enable Normalmap", &m_EnableNormalMap);

        static int skybox_item = 0;
        static const char* skybox_strs[] = {
            "Daylight",
            "Sunset",
            "Desert",
            "TextSkybox"
        };
        if (ImGui::Combo("Skybox", &skybox_item, skybox_strs, ARRAYSIZE(skybox_strs)))
        {
            Model* pModel = m_ModelManager.GetModel("Skybox");
            switch (skybox_item)
            {
            case 0: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));
                pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
                break;
            case 1: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Sunset"));
                pModel->materials[0].Set<std::string>("$Skybox", "Sunset");
                break;
            case 2: 
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Desert")); 
                pModel->materials[0].Set<std::string>("$Skybox", "Desert");
                break;
            case 3:
                m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Text"));
                pModel->materials[0].Set<std::string>("$Skybox", "Text");
                break;
            }
        }
       //调试数据
        auto CarPos = CarTransform.GetPosition();
        auto CameraPos = m_pCamera->GetPosition();
        auto CarRotation = CarTransform.GetRotation();
        auto LightDir1 = dirLight[0].direction;
        auto LightDir2 = dirLight[1].direction;
        auto LightDir3 = dirLight[2].direction;
        auto LightDir4 = dirLight[3].direction;
        ImGui::Text("Car Pos:\n x:%.2f y:%.2f z:%.2f", CarPos.x, CarPos.y, CarPos.z);
        ImGui::Text("Camera Pos:\n x:%.2f y:%.2f z:%.2f", CameraPos.x, CameraPos.y, CameraPos.z);
        ImGui::Text("Car Rotate:\n x:%.2f y:%.2f z:%.2f", CarRotation.x, CarRotation.y, CarRotation.z);
        ImGui::Text("Light Dir1:\n x:%.2f y:%.2f z:%.2f", LightDir1.x,LightDir1.y,LightDir1.z);
        ImGui::Text("Light Dir2:\n x:%.2f y:%.2f z:%.2f", LightDir2.x, LightDir2.y, LightDir2.z);
        ImGui::Text("Light Dir3:\n x:%.2f y:%.2f z:%.2f", LightDir3.x, LightDir3.y, LightDir3.z);
        ImGui::Text("Light Dir4:\n x:%.2f y:%.2f z:%.2f", LightDir4.x, LightDir4.y, LightDir4.z);
        ImGui::Text("Car velocity:\n %.2f km/h",v0);
    }
    ImGui::End();
    ImGui::Render();
    
   
}

void GameApp::DrawScene()
{
    // 创建后备缓冲区的渲染目标视图
    if (m_FrameCount < m_BackBufferCount)
    {
        ComPtr<ID3D11Texture2D> pBackBuffer;
        m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(pBackBuffer.GetAddressOf()));
        CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(D3D11_RTV_DIMENSION_TEXTURE2D, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, m_pRenderTargetViews[m_FrameCount].ReleaseAndGetAddressOf());
    }

    float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_pd3dImmediateContext->ClearRenderTargetView(GetBackBufferRTV(), black);
    m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthTexture->GetDepthStencil(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    ID3D11RenderTargetView* pRTVs[1] = { GetBackBufferRTV() };
    m_pd3dImmediateContext->OMSetRenderTargets(1, pRTVs, m_pDepthTexture->GetDepthStencil());
    D3D11_VIEWPORT viewport = m_pCamera->GetViewPort();
    m_pd3dImmediateContext->RSSetViewports(1, &viewport);
   

    // 绘制模型
    if (m_EnableNormalMap)
        m_BasicEffect.SetRenderWithNormalMap();
    else
        m_BasicEffect.SetRenderDefault();

    for (i = 0;i < 15;i++)
    {
        m_LeftTree[i].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
        m_RightTree[i].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    }
    //m_BasicEffect.SetReflectionEnabled(true);
    //m_Sphere.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    
    //m_BasicEffect.SetReflectionEnabled(false);
    m_Ground.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_GroundSide[0].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_GroundSide[1].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    m_Car.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    for (i = 0;i < 20;i++)
    {
        m_LeftBarriers[i].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
        m_RightBarriers[i].Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);
    }

    // 绘制天空盒
    m_SkyboxEffect.SetRenderDefault();
    m_Skybox.Draw(m_pd3dImmediateContext.Get(), m_SkyboxEffect);
    

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, m_IsDxgiFlipModel ? DXGI_PRESENT_ALLOW_TEARING : 0));
}

bool GameApp::InitResource()
{
    // ******************
    // 初始化天空盒相关
    
    ComPtr<ID3D11Texture2D> pTex;
    D3D11_TEXTURE2D_DESC texDesc;
    std::string filenameStr;
    std::vector<ID3D11ShaderResourceView*> pCubeTextures;
    std::unique_ptr<TextureCube> pTexCube;
    // Daylight
    {
        filenameStr = "..\\Texture\\daylight0.png";
        for (size_t i = 0; i < 6; ++i)
        {
            filenameStr[19] = '0' + (char)i;
            pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
        }

        pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        pTex->GetDesc(&texDesc);
        pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        pTexCube->SetDebugObjectName("Daylight");
        for (uint32_t i = 0; i < 6; ++i)
        {
            pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
            m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
        }
        m_TextureManager.AddTexture("Daylight", pTexCube->GetShaderResource());
    }
    
    // Sunset
    {
        filenameStr = "..\\Texture\\sunset0.bmp";
        pCubeTextures.clear();
        for (size_t i = 0; i < 6; ++i)
        {
            filenameStr[17] = '0' + (char)i;
            pCubeTextures.push_back(m_TextureManager.CreateFromFile(filenameStr));
        }
        pCubeTextures[0]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
        pTex->GetDesc(&texDesc);
        pTexCube = std::make_unique<TextureCube>(m_pd3dDevice.Get(), texDesc.Width, texDesc.Height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
        pTexCube->SetDebugObjectName("Sunset");
        for (uint32_t i = 0; i < 6; ++i)
        {
            pCubeTextures[i]->GetResource(reinterpret_cast<ID3D11Resource**>(pTex.ReleaseAndGetAddressOf()));
            m_pd3dImmediateContext->CopySubresourceRegion(pTexCube->GetTexture(), D3D11CalcSubresource(0, i, 1), 0, 0, 0, pTex.Get(), 0, nullptr);
        }
        m_TextureManager.AddTexture("Sunset", pTexCube->GetShaderResource());
    }
    // Desert
    m_TextureManager.AddTexture("Desert", m_TextureManager.CreateFromFile("..\\Texture\\desertcube1024.dds", false, true));
    // TextSkybox
    m_TextureManager.AddTexture("Text", m_TextureManager.CreateFromFile("..\\Texture\\TextSkybox02.dds", false, true));

    m_BasicEffect.SetTextureCube(m_TextureManager.GetTexture("Daylight"));

    
    // ******************
    // 初始化游戏对象
    //
    
    // 车道
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("Ground", Geometry::CreatePlane(XMFLOAT2(20.0f, 200.0f), XMFLOAT2(1.0f, 1.0f)));
        pModel->SetDebugObjectName("Ground");
        m_TextureManager.CreateFromFile("..\\Texture\\track.dds");
        m_TextureManager.CreateFromFile("..\\Texture\\trackNor3.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\track.dds");
        pModel->materials[0].Set<std::string>("$Normal", "..\\Texture\\trackNor3.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_Ground.SetModel(pModel);
        m_Ground.GetTransform().SetPosition(0.0f, -3.0f, 0.0f);
    }
    //路边草地
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("GroundSide1", Geometry::CreatePlane(XMFLOAT2(20.0f, 200.0f), XMFLOAT2(5.0f, 5.0f)));
        pModel->SetDebugObjectName("GroundSide1");
        m_TextureManager.CreateFromFile("..\\Texture\\grass.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\grass.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_GroundSide[0].SetModel(pModel);
        m_GroundSide[0].GetTransform().SetPosition(-20.0f, -3.0f, 0.0f);
    }

    {
        Model* pModel = m_ModelManager.CreateFromGeometry("GroundSide2", Geometry::CreatePlane(XMFLOAT2(20.0f, 200.0f), XMFLOAT2(5.0f, 5.0f)));
        pModel->SetDebugObjectName("GroundSide2");
        m_TextureManager.CreateFromFile("..\\Texture\\grass.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\grass.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_GroundSide[1].SetModel(pModel);
        m_GroundSide[1].GetTransform().SetPosition(20.0f, -3.0f, 0.0f);
    }
    //道路两侧围栏
    for(i = 0;i < 20;i++)
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("BarrierLeft", Geometry::CreateBox(2.0f, 2.0f, 10.0f));
        pModel->SetDebugObjectName("BarrierLeft");
        m_TextureManager.CreateFromFile("..\\Texture\\WireFence.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\WireFence.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.4f, 0.4f, 0.3f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.7f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_LeftBarriers[i].SetModel(pModel);
        m_LeftBarriers[i].GetTransform().SetPosition(12.0f, -2.99f, (float)(i - 9) * 10);
    }
    for(j = 0;j < 20;j++)
    {
        Model* pModel = m_ModelManager.CreateFromGeometry("BarrierRight", Geometry::CreateBox(2.0f, 2.0f, 10.0f));
        pModel->SetDebugObjectName("BarrierRight");
        m_TextureManager.CreateFromFile("..\\Texture\\WireFence.dds");
        pModel->materials[0].Set<std::string>("$Diffuse", "..\\Texture\\WireFence.dds");
        pModel->materials[0].Set<XMFLOAT4>("$AmbientColor", XMFLOAT4(0.4f, 0.4f, 0.3f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$DiffuseColor", XMFLOAT4(0.5f, 0.7f, 0.6f, 1.0f));
        pModel->materials[0].Set<XMFLOAT4>("$SpecularColor", XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f));
        pModel->materials[0].Set<float>("$SpecularPower", 16.0f);
        pModel->materials[0].Set<XMFLOAT4>("$ReflectColor", XMFLOAT4());
        m_RightBarriers[j].SetModel(pModel);
        m_RightBarriers[j].GetTransform().SetPosition(-12.0f, -2.99f, (float)(j - 9) * 10);
    }
    //路边树木
    for(i = 0;i < 15;i++)
    {
        Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\tree.obj");    
        pModel->SetDebugObjectName("tree");
        m_LeftTree[i].SetModel(pModel);
        m_LeftTree[i].GetTransform().SetPosition(-15.0f, -2.99f, (float)(i - 9) * 10);

        Transform& treeTransform = m_LeftTree[i].GetTransform();
        treeTransform.SetScale(0.01f, 0.01f, 0.01f);
    }

    for (j = 0;j < 15;j++)
    {
        Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\tree.obj");
        pModel->SetDebugObjectName("tree");
        m_RightTree[j].SetModel(pModel);
        m_RightTree[j].GetTransform().SetPosition(15.0f, -2.99f, (float)(j - 9) * 10);

        Transform& treeTransform = m_RightTree[j].GetTransform();
        treeTransform.SetScale(0.01f, 0.01f, 0.01f);
    }
    //小车
    {
        Model* pModel = m_ModelManager.CreateFromFile("..\\Model\\Cybertruck2.obj");
        pModel->SetDebugObjectName("Car");  
        m_Car.SetModel(pModel);
        m_Car.GetTransform().SetPosition(0.0f,-2.99f,-100.0f);
        m_Car.GetTransform().Rotate(XMFLOAT3(0.0f, XM_PIDIV2, 0.0f));


        Transform& CarTransform = m_Car.GetTransform();
        CarTransform.SetScale(2.0f, 2.0f, 2.0f);
    }

    // 天空盒立方体
    Model* pModel = m_ModelManager.CreateFromGeometry("Skybox", Geometry::CreateBox());
    pModel->SetDebugObjectName("Skybox");
    pModel->materials[0].Set<std::string>("$Skybox", "Daylight");
    m_Skybox.SetModel(pModel);


    // ******************
    
    // 初始化摄像机
    
    //1.玩家摄像机
    auto camera = std::make_shared<FirstPersonCamera>();
    m_pCamera = camera;
    m_CameraController.InitCamera(camera.get());
    camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
    camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
    camera->LookTo(XMFLOAT3(0.0f, -2.99f, -100.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
    m_CameraMode = CameraMode::FirstPerson;


    m_BasicEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_BasicEffect.SetProjMatrix(camera->GetProjMatrixXM());
    m_SkyboxEffect.SetViewMatrix(camera->GetViewMatrixXM());
    m_SkyboxEffect.SetProjMatrix(camera->GetProjMatrixXM());

    // ******************
    // 初始化不会变化的值
    //  

    //// 方向光
    //DirectionalLight dirLight[4]{};
    //dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
    //dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    //dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    //dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
    //dirLight[1] = dirLight[0];
    //dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);
    //dirLight[2] = dirLight[0];
    //dirLight[2].direction = XMFLOAT3(0.577f, -0.577f, -0.577f);
    //dirLight[3] = dirLight[0];
    //dirLight[3].direction = XMFLOAT3(-0.577f, -0.577f, -0.577f);
    //for (int i = 0; i < 4; ++i)    
    //    m_BasicEffect.SetDirLight(i, dirLight[i]);
    
    return true;
}


