#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <Vertex.h>
#include <Geometry.h>
#include <Transform.h>
#include <Texture2D.h>
#include <GameObject.h>
#include <ModelManager.h>
#include <EffectHelper.h>
#include "Effects.h"

class FFTWaveCreator
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

    void SetMaterial(const Material& material);

    Transform& GetTransform();
    const Transform& GetTransform() const;

    UINT RowCount() const;
    UINT ColumnCount() const;

    struct FFTWave
    {

        float g_WaveＷ;     // 角频率
        float g_WaveＫ;     // 波长
        float g_WaveＸ;     // 海平面的大小
        float g_WaveＹ;     // 海平面的大小
        float g_WaveR;      //高斯随机数
        float g_WaveI;      //高斯随机数

        DirectX::XMFLOAT2 g_angle;     // 风向的角度
        DirectX::XMFLOAT2 WaveD;    // 方向
        DirectX::XMFLOAT2 pad;      // 打包
    };

public:
    FFTWaveCreator() = default;
    ~FFTWaveCreator() = default;
    //禁止拷贝，可移动语义
    FFTWaveCreator(const FFTWaveCreator&) = delete;
    FFTWaveCreator& operator= (const FFTWaveCreator&) = delete;
    FFTWaveCreator(FFTWaveCreator&&) = default;
    FFTWaveCreator& operator= (FFTWaveCreator&&) = default;

    HRESULT InitResource(ID3D11Device* device,
        const std::wstring& texFileName,  // 纹理文件名
        UINT rows,                          // 顶点行数
        UINT cols,                          // 顶点列数
        float texU,                          // 纹理坐标U方向最大值
        float texV,                          // 纹理坐标V方向最大值
        float timeStep,                      // 时间步长
        float spatialStep,                  // 空间步长
        FFTWave* FFTData        // 数据
    );

    // 设置数据
    void SetData(FFTWave* FFTData);

    // 更新
    void Update(ID3D11DeviceContext* deviceContext, float t);

    // 绘制
    void Draw(ID3D11DeviceContext* deviceContext, IEffect& effect);

    // 设置DeBug名称
    void SetDebugObjectName(const std::string& name);

private:

    void Init(
        UINT rows,                    // 顶点行数
        UINT cols,                    // 顶点列数
        float texU,                    // 纹理坐标U方向最大值
        float texV,                    // 纹理坐标V方向最大值
        float timeStep,                // 时间步长
        float spatialStep,             // 空间步长
        FFTWave* FFTData  // 数据
    );

    UINT m_NumRows = 0;                    // 顶点行数
    UINT m_NumCols = 0;                    // 顶点列数

    UINT m_VertexCount = 0;                // 顶点数目
    UINT m_IndexCount = 0;                // 索引数目

    Transform m_Transform = {};            // 水面变换
    DirectX::XMFLOAT2 m_TexOffset = {};    // 纹理坐标偏移
    float m_TexU = 0.0f;                // 纹理坐标U方向最大值
    float m_TexV = 0.0f;                // 纹理坐标V方向最大值
    Material m_Material = {};            // 水面材质

    FFTWave m_FFTwaveData[3] = {};

    float m_TimeStep = 0.0f;            // 时间步长
    float m_SpatialStep = 0.0f;            // 空间步长
    float m_AccumulateTime = 0.0f;        // 累积时间
    float m_TotalTime = 0.0f;           // 总时长

private:

    ComPtr<ID3D11Buffer> m_pCurrVertex;                        // 保存当前模拟结果的顶点
    ComPtr<ID3D11UnorderedAccessView> m_pCurrVertexUAV;        // 缓存当前模拟结果的顶点 无序访问视图

    ComPtr<ID3D11Buffer> m_pVertex;                            // 初始顶点 缓冲区
    ComPtr<ID3D11UnorderedAccessView> m_pVertexUAV;            // 初始顶点 无序访问视图

    ComPtr<ID3D11Buffer> m_pVertexBuffer;                    // 顶点缓冲区
    ComPtr<ID3D11Buffer> m_pIndexBuffer;                    // 索引缓冲区
    ComPtr<ID3D11Buffer> m_pConstantBuffer;                    // 常量缓冲区

    ComPtr<ID3D11Buffer> m_pTempBuffer;                     // 用于顶点数据拷贝的缓冲区

    ComPtr<ID3D11ComputeShader> m_pWavesUpdateCS;            // 用于计算模拟结果的着色器

    ComPtr<ID3D11ShaderResourceView> m_pTextureDiffuse;        // 水面纹理
   
    //用Comptr管理纹理资源
    ComPtr<ID3D11Texture2D> m_pGaussianRandomRT;         //高斯随机数对
    ComPtr<ID3D11Texture2D> m_pHeightSpectrumRT;         //高度频谱
    ComPtr<ID3D11Texture2D> m_pDisplaceXSpectrumRT;      //x方向偏移频谱
    ComPtr<ID3D11Texture2D> m_pDisplaceZSpectrumRT;      //z方向偏移频谱
    ComPtr<ID3D11Texture2D> m_pDisplaceRT;               //（总）偏移频谱
    ComPtr<ID3D11Texture2D> m_pInputRT;                  //FFT输入纹理
    ComPtr<ID3D11Texture2D> m_pOutputRT;                 //FFT输出纹理
    ComPtr<ID3D11Texture2D> m_pNormalRT;                 //法线纹理
    ComPtr<ID3D11Texture2D> m_pBubblesRT;                //泡沫纹理
    
    //只需要传递一次的cb
    struct {
        float m_SpatialStep = 0.0f;			// 空间步长
        int fftSize;                        //fft纹理大小 
        DirectX::XMFLOAT2 g_pad1;
    } m_CBInitSettings = {};
   
    struct {

        FFTWave FFTData[3];

        float TotalTime;    // 总时长
        float GroundCountX; // X方向上的线程团数
        DirectX::XMFLOAT2 Pad;

    } m_CBUpdateSettings = {};

    //专门传递Ns的结构体（用于FFT计算时的阶数）
    struct {
        int ns;
        DirectX::XMFLOAT3 g_pad3;
    }m_CBns = {};
};