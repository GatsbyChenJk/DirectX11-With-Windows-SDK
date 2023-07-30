#include "FFTWaves.hlsli"
//生成偏移频谱
[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    float2 k = float2(2 * PI * id.x / N - PI, 2 * PI * id.y / N - PI);
    k /= max(0.001f, length(k));
    float2 HTilde = HeightSpectrumRT[id.xy].xy;

    float2 KxHTilde = complexMultiply(float2(0, -k.x), HTilde);
    float2 kzHTilde = complexMultiply(float2(0, -k.y), HTilde);

    DisplaceXSpectrumRT[id.xy] = float4(KxHTilde, 0, 0);
    DisplaceZSpectrumRT[id.xy] = float4(kzHTilde, 0, 0);
}
