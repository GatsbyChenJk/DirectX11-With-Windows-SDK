#include "FFTWaves.hlsli"
//生成法线和泡沫纹理
[numthreads(16, 16, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    //计算法线
    float uintLength = OceanLength / (N - 1.0f); //两点间单位长度
    //获取当前点，周围4个点的uv坐标
    uint2 uvX1 = uint2((id.x - 1.0f + N) % N, id.y);
    uint2 uvX2 = uint2((id.x + 1.0f + N) % N, id.y);
    uint2 uvZ1 = uint2(id.x, (id.y - 1.0f + N) % N);
    uint2 uvZ2 = uint2(id.x, (id.y + 1.0f + N) % N);

    //以当前点为中心，获取周围4个点的偏移值
    float3 x1D = DisplaceRT[uvX1].xyz; //在x轴 第一个点的偏移值
    float3 x2D = DisplaceRT[uvX2].xyz; //在x轴 第二个点的偏移值
    float3 z1D = DisplaceRT[uvZ1].xyz; //在z轴 第一个点的偏移值
    float3 z2D = DisplaceRT[uvZ2].xyz; //在z轴 第二个点的偏移值

    //以当前点为原点，构建周围4个点的坐标
    float3 x1 = float3(x1D.x - uintLength, x1D.yz); //在x轴 第一个点的坐标
    float3 x2 = float3(x2D.x + uintLength, x2D.yz); //在x轴 第二个点的坐标
    float3 z1 = float3(z1D.xy, z1D.z - uintLength); //在z轴 第一个点的坐标
    float3 z2 = float3(z1D.xy, z1D.z + uintLength); //在z轴 第二个点的坐标

    //计算两个切向量
    float3 tangentX = x2 - x1;
    float3 tangentZ = z2 - z1;

    //计算法线
    float3 normal = normalize(cross(tangentZ, tangentX));

    //计算泡沫
    float3 ddx = x2D - x1D;
    float3 ddz = z2D - z1D;
    //雅可比行列式
    float jacobian = (1.0f + ddx.x) * (1.0f + ddz.z) - ddx.z * ddz.x;

    jacobian = saturate(max(0, BubblesThreshold - saturate(jacobian)) * BubblesScale);

    NormalRT[id.xy] = float4(normal, 0);
    BubblesRT[id.xy] = float4(jacobian, jacobian, jacobian, 0);
}
