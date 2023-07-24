#include "Tessellation.hlsli"

float4 VS( VertexOut vIn : POSITION ) : POSITION
{
	return vIn.posL;
}