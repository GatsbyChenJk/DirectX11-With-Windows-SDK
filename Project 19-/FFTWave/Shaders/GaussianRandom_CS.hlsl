#include "FFTWaves.hlsli"

[numthreads(16, 16, 1)]
void CS( uint3 id : SV_DispatchThreadID )
{
    float2 gau = gaussian(id.xy);

    GaussianRandomRT[id.xy] = float4(gau, 0, 0);

}