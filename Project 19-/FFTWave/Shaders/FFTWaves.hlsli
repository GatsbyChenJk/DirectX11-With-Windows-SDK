#define GroundThreadSize 16
#define WaveCount 3
 
static const float PI = 3.141592653f;
static const float g = 9.84f;

//struct FFTWave
//{
//    float g_WaveW; // 角频率 
//    float g_WaveK; // 波长 
//    float g_WaveX; // 海平面的大小 
//    float g_WaveY; // 海平面的大小
//    float g_WaveR; //高斯随机数
//    float g_WaveI; //高斯随机数 
//    float2 g_angle; // 风向的角度
 
//    float2 g_WaveD; // 方向
//    float2 g_pad; // 打包 
//};
 
struct VertexPosNormalTex
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};
 
//RW纹理
RWTexture2D<float4> GaussianRandomRT : register(u0); //高斯随机数
RWTexture2D<float4> HeightSpectrumRT : register(u1); //高度频谱
RWTexture2D<float4> DisplaceXSpectrumRT : register(u2); //X偏移频谱
RWTexture2D<float4> DisplaceZSpectrumRT : register(u3); //Z偏移频谱
RWTexture2D<float4> DisplaceRT : register(u4); //最后生成的偏移纹理
RWTexture2D<float4> InputRT : register(u5); //输入
RWTexture2D<float4> OutputRT : register(u6); //输出
RWTexture2D<float4> NormalRT : register(u7); //法线纹理
RWTexture2D<float4> BubblesRT : register(u8); //泡沫纹理

// 用于初始化的cb
cbuffer cbInitSettings : register(b0)
{
    float OceanLength; //海洋长度
    int N; //fft纹理大小
    //uint rngState;
    float2 g_pad1;
}

// 用于更新模拟的cb
cbuffer cbUpdataSettings : register(b1)
{
    float4 WindAndSeed; //风和随机种子 xy为风, zw为两个随机种子
    
    float Lambda; //偏移影响
    float HeightScale; //高度影响
    float BubblesScale; //泡沫强度
    float BubblesThreshold; //泡沫阈值
    
    float A; //phillips谱参数，影响波浪高度
    float Time; //时间
    float2 g_pad2;
}


//用于FFT计算的阶数
cbuffer cbns : register(b2)
{
    int Ns;
    float3 g_pad3;
}

//---------------------------------------------------------
//   工具函数
//---------------------------------------------------------
//随机种子
uint wangHash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
//计算均匀分布随机数[0,1)
float rand(uint rngState)
{
    // Xorshift算法
    rngState ^= (rngState << 13);
    rngState ^= (rngState >> 17);
    rngState ^= (rngState << 5);
    return rngState / 4294967296.0f;;
}
//计算高斯随机数
float2 gaussian(float2 id)
{
    //均匀分布随机数
    uint rngState = wangHash(id.y * N + id.x);
    float x1 = rand(rngState);
    float x2 = rand(rngState);
 
    x1 = max(1e-6f, x1);
    x2 = max(1e-6f, x2);
    //计算两个相互独立的高斯随机数
    float g1 = sqrt(-2.0f * log(x1)) * cos(2.0f * PI * x2);
    float g2 = sqrt(-2.0f * log(x1)) * sin(2.0f * PI * x2);
 
    return float2(g1, g2);
}
//计算弥散
float dispersion(float2 k)
{
    return sqrt(g * length(k));
}
//复数相乘
float2 complexMultiply(float2 c1, float2 c2)
{
    return float2(c1.x * c2.x - c1.y * c2.y,
    c1.x * c2.y + c1.y * c2.x);
}
//计算phillips谱
float phillips(float2 k)
{
    float kLength = length(k);
    kLength = max(0.001f, kLength);
    // kLength = 1;
    float kLength2 = kLength * kLength;
    float kLength4 = kLength2 * kLength2;

    float windLength = length(WindAndSeed.xy);
    float l = windLength * windLength / g;
    float l2 = l * l;

    float damping = 0.001f;
    float L2 = l2 * damping * damping;
    float A = kLength;

    //phillips谱
    return A * exp(-1.0f / (kLength2 * l2)) / kLength4 * exp(-kLength2 * L2);
}
//Donelan-Banner方向拓展
float DonelanBannerDirectionalSpreading(float2 k)
{
    float betaS;
    float omegap = 0.855f * g / length(WindAndSeed.xy);
    float ratio = dispersion(k) / omegap;

    if (ratio < 0.95f)
    {
        betaS = 2.61f * pow(ratio, 1.3f);
    }
    if (ratio >= 0.95f && ratio < 1.6f)
    {
        betaS = 2.28f * pow(ratio, -1.3f);
    }
    if (ratio > 1.6f)
    {
        float epsilon = -0.4f + 0.8393f * exp(-0.567f * log(ratio * ratio));
        betaS = pow(10, epsilon);
    }
    float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);

    return betaS / max(1e-7f, 2.0f * tanh(betaS * PI) * pow(cosh(betaS * theta), 2));
}
//正余弦平方方向拓展
float PositiveCosineSquaredDirectionalSpreading(float2 k)
{
    float theta = atan2(k.y, k.x) - atan2(WindAndSeed.y, WindAndSeed.x);
    if (theta > -PI / 2.0f && theta < PI / 2.0f)
    {
        return 2.0f / PI * pow(cos(theta), 2);
    }
    else
    {
        return 0;
    }
}




