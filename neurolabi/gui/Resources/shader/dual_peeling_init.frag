//--------------------------------------------------------------------------------------
// Order Independent Transparency with Dual Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

void fragment_func(out vec4 fragColor0, out vec4 fragColor1, out float fragDepth);

void main(void)
{
	float fragDepth;
	vec4 color;
        fragment_func(color, vec4(0.0,0.0,0.0,0.0), fragDepth);
	FragData0.xy = vec2(-fragDepth, fragDepth);
	FragData1.x = -fragDepth;
}
