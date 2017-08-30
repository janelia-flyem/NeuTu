//--------------------------------------------------------------------------------------
// Order Independent Transparency with Dual Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
layout(location = 1) out vec4 FragData1;
#elif GLSL_VERSION >= 130
out vec4 FragData0;
out vec4 FragData1;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#define FragData1 gl_FragData[1]
#endif

void fragment_func(out vec4 fragColor, out float fragDepth);

void main(void)
{
	float fragDepth;
	vec4 color;
	fragment_func(color, fragDepth);
  gl_FragDepth = fragDepth;
	FragData0.xy = vec2(-fragDepth, fragDepth);
	FragData1.x = -fragDepth;
}
