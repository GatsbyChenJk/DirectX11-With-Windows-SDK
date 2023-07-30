#include "FFTWaves.hlsli"

[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    int2 idxs = id.xy;
    idxs.y = floor(id.y / (Ns * 2.0f)) * Ns + id.y % Ns;
    float angle = 2.0f * PI * (id.y / (Ns * 2.0f));
    float2 w = float2(cos(angle), sin(angle));

    float2 x0 = InputRT[idxs].xy;
    float2 x1 = InputRT[int2(idxs.x, idxs.y + N * 0.5f)].xy;

    float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
    OutputRT[id.xy] = float4(output, 0, 0);
}