#include "FFTWaves.hlsli"

//生成高度频谱
[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    float2 k = float2(2.0f * PI * id.x / N - PI, 2.0f * PI * id.y / N - PI);

    float2 gaussian = GaussianRandomRT[id.xy].xy;

    float2 hTilde0 = gaussian * sqrt(abs(phillips(k) * DonelanBannerDirectionalSpreading(k)) / 2.0f);
    float2 hTilde0Conj = gaussian * sqrt(abs(phillips(-k) * DonelanBannerDirectionalSpreading(-k)) / 2.0f);
    hTilde0Conj.y *= -1.0f;

    float omegat = dispersion(k) * Time;
    float c = cos(omegat);
    float s = sin(omegat);
    
    float2 h1 = complexMultiply(hTilde0, float2(c, s));
    float2 h2 = complexMultiply(hTilde0Conj, float2(c, -s));

    float2 HTilde = h1 + h2;

    HeightSpectrumRT[id.xy] = float4(HTilde, 0, 0);
}
