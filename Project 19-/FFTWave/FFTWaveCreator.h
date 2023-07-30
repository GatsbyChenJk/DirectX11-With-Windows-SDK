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

        float g_Wave��;     // ��Ƶ��
        float g_Wave��;     // ����
        float g_Wave��;     // ��ƽ��Ĵ�С
        float g_Wave��;     // ��ƽ��Ĵ�С
        float g_WaveR;      //��˹�����
        float g_WaveI;      //��˹�����

        DirectX::XMFLOAT2 g_angle;     // ����ĽǶ�
        DirectX::XMFLOAT2 WaveD;    // ����
        DirectX::XMFLOAT2 pad;      // ���
    };

public:
    FFTWaveCreator() = default;
    ~FFTWaveCreator() = default;
    //��ֹ���������ƶ�����
    FFTWaveCreator(const FFTWaveCreator&) = delete;
    FFTWaveCreator& operator= (const FFTWaveCreator&) = delete;
    FFTWaveCreator(FFTWaveCreator&&) = default;
    FFTWaveCreator& operator= (FFTWaveCreator&&) = default;

    HRESULT InitResource(ID3D11Device* device,
        const std::wstring& texFileName,  // �����ļ���
        UINT rows,                          // ��������
        UINT cols,                          // ��������
        float texU,                          // ��������U�������ֵ
        float texV,                          // ��������V�������ֵ
        float timeStep,                      // ʱ�䲽��
        float spatialStep,                  // �ռ䲽��
        FFTWave* FFTData        // ����
    );

    // ��������
    void SetData(FFTWave* FFTData);

    // ����
    void Update(ID3D11DeviceContext* deviceContext, float t);

    // ����
    void Draw(ID3D11DeviceContext* deviceContext, IEffect& effect);

    // ����DeBug����
    void SetDebugObjectName(const std::string& name);

private:

    void Init(
        UINT rows,                    // ��������
        UINT cols,                    // ��������
        float texU,                    // ��������U�������ֵ
        float texV,                    // ��������V�������ֵ
        float timeStep,                // ʱ�䲽��
        float spatialStep,             // �ռ䲽��
        FFTWave* FFTData  // ����
    );

    UINT m_NumRows = 0;                    // ��������
    UINT m_NumCols = 0;                    // ��������

    UINT m_VertexCount = 0;                // ������Ŀ
    UINT m_IndexCount = 0;                // ������Ŀ

    Transform m_Transform = {};            // ˮ��任
    DirectX::XMFLOAT2 m_TexOffset = {};    // ��������ƫ��
    float m_TexU = 0.0f;                // ��������U�������ֵ
    float m_TexV = 0.0f;                // ��������V�������ֵ
    Material m_Material = {};            // ˮ�����

    FFTWave m_FFTwaveData[3] = {};

    float m_TimeStep = 0.0f;            // ʱ�䲽��
    float m_SpatialStep = 0.0f;            // �ռ䲽��
    float m_AccumulateTime = 0.0f;        // �ۻ�ʱ��
    float m_TotalTime = 0.0f;           // ��ʱ��

private:

    ComPtr<ID3D11Buffer> m_pCurrVertex;                        // ���浱ǰģ�����Ķ���
    ComPtr<ID3D11UnorderedAccessView> m_pCurrVertexUAV;        // ���浱ǰģ�����Ķ��� ���������ͼ

    ComPtr<ID3D11Buffer> m_pVertex;                            // ��ʼ���� ������
    ComPtr<ID3D11UnorderedAccessView> m_pVertexUAV;            // ��ʼ���� ���������ͼ

    ComPtr<ID3D11Buffer> m_pVertexBuffer;                    // ���㻺����
    ComPtr<ID3D11Buffer> m_pIndexBuffer;                    // ����������
    ComPtr<ID3D11Buffer> m_pConstantBuffer;                    // ����������

    ComPtr<ID3D11Buffer> m_pTempBuffer;                     // ���ڶ������ݿ����Ļ�����

    ComPtr<ID3D11ComputeShader> m_pWavesUpdateCS;            // ���ڼ���ģ��������ɫ��

    ComPtr<ID3D11ShaderResourceView> m_pTextureDiffuse;        // ˮ������
   
    //��Comptr����������Դ
    ComPtr<ID3D11Texture2D> m_pGaussianRandomRT;         //��˹�������
    ComPtr<ID3D11Texture2D> m_pHeightSpectrumRT;         //�߶�Ƶ��
    ComPtr<ID3D11Texture2D> m_pDisplaceXSpectrumRT;      //x����ƫ��Ƶ��
    ComPtr<ID3D11Texture2D> m_pDisplaceZSpectrumRT;      //z����ƫ��Ƶ��
    ComPtr<ID3D11Texture2D> m_pDisplaceRT;               //���ܣ�ƫ��Ƶ��
    ComPtr<ID3D11Texture2D> m_pInputRT;                  //FFT��������
    ComPtr<ID3D11Texture2D> m_pOutputRT;                 //FFT�������
    ComPtr<ID3D11Texture2D> m_pNormalRT;                 //��������
    ComPtr<ID3D11Texture2D> m_pBubblesRT;                //��ĭ����
    
    //ֻ��Ҫ����һ�ε�cb
    struct {
        float m_SpatialStep = 0.0f;			// �ռ䲽��
        int fftSize;                        //fft�����С 
        DirectX::XMFLOAT2 g_pad1;
    } m_CBInitSettings = {};
   
    struct {

        FFTWave FFTData[3];

        float TotalTime;    // ��ʱ��
        float GroundCountX; // X�����ϵ��߳�����
        DirectX::XMFLOAT2 Pad;

    } m_CBUpdateSettings = {};

    //ר�Ŵ���Ns�Ľṹ�壨����FFT����ʱ�Ľ�����
    struct {
        int ns;
        DirectX::XMFLOAT3 g_pad3;
    }m_CBns = {};
};