#include "FFTWaves.hlsli"
//横向FFT最后阶段计算,需要进行特别处理
[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    int2 idxs = id.xy;
    idxs.x = floor(id.x / (Ns * 2.0f)) * Ns + id.x % Ns;
    float angle = 2.0f * PI * (id.x / (Ns * 2.0f));
    float2 w = float2(cos(angle), sin(angle));

    /*********不同点***********/
    w *= -1;
    /***************************/

    float2 x0 = InputRT[idxs].xy;
    float2 x1 = InputRT[int2(idxs.x + N * 0.5f, idxs.y)].xy;

    float2 output = x0 + float2(w.x * x1.x - w.y * x1.y, w.x * x1.y + w.y * x1.x);
    /*********不同点***********/
    int x = id.x - N * 0.5f;
    output *= ((x + 1) % 2.0f) * 1 + (x % 2.0f) * (-1);
    /***************************/
    OutputRT[id.xy] = float4(output, 0, 0);
}
