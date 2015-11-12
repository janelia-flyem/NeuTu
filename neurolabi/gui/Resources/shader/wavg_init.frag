//--------------------------------------------------------------------------------------
// Order Independent Transparency with Average Color
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//#extension ARB_draw_buffers : require

void fragment_func(out vec4 fragColor0, out vec4 fragColor1, out float fragDepth);

void main(void)
{
	float fragDepth;
	vec4 color;
        fragment_func(color, vec4(0.0, 0.0, 0.0, 0.0), fragDepth);
	FragData0 = color;
	FragData1.xy = vec2(1.0, fragDepth);
}
