#include "FFTWaves.hlsli"
//生成偏移纹理
[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    float y = length(HeightSpectrumRT[id.xy].xy) / (N * N) * HeightScale; //高度
    float x = length(DisplaceXSpectrumRT[id.xy].xy) / (N * N) * Lambda; //x轴偏移
    float z = length(DisplaceZSpectrumRT[id.xy].xy) / (N * N) * Lambda; //z轴偏移
    
    HeightSpectrumRT[id.xy] = float4(y, y, y, 0);
    DisplaceXSpectrumRT[id.xy] = float4(x, x, x, 0);
    DisplaceZSpectrumRT[id.xy] = float4(z, z, z, 0);
    DisplaceRT[id.xy] = float4(x, y, z, 0);
}
