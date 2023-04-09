# 一 准备工作
### 1.创建你自己的项目
首先生成imgui的库文件，可以在imgui官网下载相应的文件，然后找到头文件和源文件的位置。
打开vs新建一个空项目，这里我将项目命名为ImGui。
### 2.添加头文件和源文件
接着打开解决方案资源管理器，按照 头文件->添加->现有项 然后会弹出文件窗口，将imgui头文件和源文件的路径复制粘贴，先将头文件选中后确认，添加好头文件 。再按照 源文件->添加->现有项 以相似的方式把源文件添加进项目里。像下面这样就是添加成功了：
![在这里插入图片描述](https://img-blog.csdnimg.cn/f314061d85dc4927bda192e57f63f1d0.png)
### 3.设置项目属性
以上两步完成后，就要生成库文件了。首先在属性窗口中的 配置属性->常规 里面设置如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/30f5b5d5b0024acfa8344acb9abd65b6.png)
需要修改成图中格式的属性有 输出目录 中间目录 配置类型 。

当以上这些设置好以后，右键创建的项目，生成即可。
生成成功后，在创建项目的路径中可以找到相应的Debug和Release版本的静态库，也就是
项目名称.lib，在我这里是ImGui.lib。
两个版本的库文件路径分别为：
ImGui/x64/Debug/ImGui.lib
ImGui/x64/Release/ImGui.lib

### 4.在DX11项目中配置imgui文件
这里我以Xjun教程项目中的02 Rendering a Triangle项目为例
首先设置好项目属性：
在 VC++目录中，在**包含目录**中添加imgui头文件的路径
在**库目录**中添加你生成的库文件的路径
在C/C++ -> 常规 中的**附加包含目录**添加你创建的imgui项目的目录
然后在 链接器->输入 里面的**附加依赖项**里面添加你的库文件名称，例如我这里生成的库文件名称是ImGui.lib。
最后在项目中的源文件里添加imgui的cpp文件，**添加完记得检查有没有漏添加哪一个文件，少了的话会报错。**


# 二 初始化imgui
首先在DX11项目中的D3DApp.h中添加下面三个预编译指令
```cpp
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
```
在D3DApp.cpp中添加外部函数

```cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
```
然后添加InitImGui函数（不要忘记在头文件中声明函数）

```cpp
bool D3DApp::InitImGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 允许键盘控制
    io.ConfigWindowsMoveFromTitleBarOnly = true;              // 仅允许标题拖动

    // 设置Dear ImGui风格
    ImGui::StyleColorsDark();

    // 设置平台/渲染器后端
    ImGui_ImplWin32_Init(m_hMainWnd);
    ImGui_ImplDX11_Init(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get());

    return true;

}

```
接着在D3DApp.cpp中的Init函数中添加以下语句

```cpp
 if (!InitImGui())
        return false;
```
然后再同一文件中的MsgProc函数添加如下语句，使得gui窗口能够接收我们的操作并处理

```cpp
if (ImGui_ImplWin32_WndProcHandler(m_hMainWnd, msg, wParam, lParam))
        return true;
```
最后再同一文件中的Run函数中调用三个函数，分别对应的是对direct3d，windows和imgui的操作

```cpp
int D3DApp::Run()
{
    MSG msg = { 0 };

    m_Timer.Reset();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_Timer.Tick();

            if (!m_AppPaused)
            {
                CalculateFrameStats();
                // 这里添加函数
                ImGui_ImplDX11_NewFrame();     
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                // --------
                UpdateScene(m_Timer.DeltaTime());
                DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

```
再到GameApp.cpp里面的DrawScene函数中调用以下函数

```cpp
void GameApp::DrawScene()
{  
    // 可以在这之前调用ImGui的UI部分
    // Direct3D 绘制部分 
    ImGui::Render();
    // 下面这句话会触发ImGui在Direct3D的绘制
    // 因此需要在此之前将后备缓冲区绑定到渲染管线上
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    HR(m_pSwapChain->Present(0, 0));
}
```
至此，imgui的基本属性已经配置好了，现在我们就可以调用imgui库里的函数来制作自己的ui界面了。

这里可以再GameApp.cpp里面的UpdateScene函数里调用下面三个函数：

```cpp
void GameApp::UpdateScene(float dt)
{
    // ImGui内部示例窗口
    ImGui::ShowAboutWindow();//展示一个“关于”窗口示例
    ImGui::ShowDemoWindow();//展示一个示例窗口
    ImGui::ShowUserGuide();//展示一个“用户说明”示例
}

```
效果如下：
![在这里插入图片描述](https://img-blog.csdnimg.cn/f1203e80ce9948d78163b4ef714af8fc.png)

