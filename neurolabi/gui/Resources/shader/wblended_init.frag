//----------------------------------------------------------------------------------
// File:        gl4-kepler\WeightedBlendedOIT\assets\shaders/weighted_blend_fragment.glsl
// SDK Version: v3.00
// Email:       gameworks@nvidia.com
// Site:        http://developer.nvidia.com/
//
// Copyright (c) 2014-2015, NVIDIA CORPORATION. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//----------------------------------------------------------------------------------

uniform float ze_to_zw_a;
uniform float ze_to_zw_b;

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

uniform float weighted_blended_depth_scale = 1;

void fragment_func(out vec4 fragColor, out float fragDepth);

void main(void)
{
  float fragDepth;
  vec4 color;
  fragment_func(color, fragDepth);
  gl_FragDepth = fragDepth;
  // calculate z from fragDepth
  //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
  // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
  float viewDepth = ze_to_zw_a / (fragDepth - ze_to_zw_b); //abs(1.0 / gl_FragCoord.w);

  // Tuned to work well with FP16 accumulation buffers and 0.0005 < viewDepth * 0.005 * weighted_blended_depth_scale < 2.5
  // See Equation (9) from http://jcgt.org/published/0002/02/09/
  float weight = clamp(0.03 / (1e-5 + pow(viewDepth * 0.005 * weighted_blended_depth_scale, 4.0)), 1e-2, 3e3);
  FragData0 = color * weight;
  FragData1.x = color.a;
}
