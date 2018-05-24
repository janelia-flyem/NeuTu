#if GLSL_VERSION < 130
uniform vec2 screen_dim_RCP;
#endif
uniform float sampling_rate;
#ifdef ISO
uniform float iso_value;
#endif
#ifdef LOCAL_MIP
uniform float local_MIP_threshold;
#endif
uniform float ze_to_zw_a;
uniform float ze_to_zw_b;

uniform sampler2D ray_entry_tex_coord;
uniform sampler2D ray_entry_eye_coord;
uniform sampler2D ray_exit_tex_coord;
uniform sampler2D ray_exit_eye_coord;

uniform sampler3D volume_1;
uniform vec3 volume_dimensions_1;
uniform sampler1D transfer_function_1;

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

vec4 compositeDVR(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth)
{
  if (rayDepth < 0.0)
    rayDepth = currentRayLength;

  vec4 result = vec4(0.0);

  result.a = curResult.a + (1.0 -curResult.a) * color.a;
  result.rgb = (curResult.rgb * curResult.a + (1.0 - curResult.a) * color.a * color.rgb) / result.a;

  return result;
}

vec4 compositeISO(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth, in float isoValue)
{
  vec4 result = curResult;
  float epsilon = 0.02;
  if (color.a >= isoValue-epsilon && color.a <= isoValue+epsilon) {
    result = color;
    result.a = 1.0;
    rayDepth = currentRayLength;
  }
  return result;
}

vec4 compositeXRay(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth)
{
  if (rayDepth < 0.0)
    rayDepth = currentRayLength;
  return curResult + color;
}

void main()
{
#if GLSL_VERSION >= 130
  vec4 entryTexCoordAndZ = texelFetch(ray_entry_tex_coord, ivec2(gl_FragCoord.xy), 0);
  vec4 exitTexCoordAndZ = texelFetch(ray_exit_tex_coord, ivec2(gl_FragCoord.xy), 0);
#else
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;
  vec4 entryTexCoordAndZ = texture2D(ray_entry_tex_coord, texCoords);
  vec4 exitTexCoordAndZ = texture2D(ray_exit_tex_coord, texCoords);
#endif
  vec3 startRayPosition = entryTexCoordAndZ.xyz;
  vec3 exitRayPosition = exitTexCoordAndZ.xyz;

  if (startRayPosition == exitRayPosition) {
    discard;   // background
  } else {
    vec4 result = vec4(0.0);

#ifdef MIP
    float ch1V = 0.0;
#endif

    vec3 rayVector = exitRayPosition - startRayPosition;
    vec3 numVoxels = abs(rayVector * volume_dimensions_1);
    float numVoxel = max(max(numVoxels.x, numVoxels.y), numVoxels.z);
    float stepSize = 1.0 / (sampling_rate * numVoxel);

    float currentRayLength = 0.0;
    float rayDepth = -1.0;
    bool finished = false;
    for (int loop0=0; !finished && loop0<255; loop0++) {
      for (int loop1=0; !finished && loop1<255; loop1++) {
        float voxel;
        vec3 samplePos = startRayPosition + currentRayLength * rayVector;

#if GLSL_VERSION >= 130
        voxel = texture(volume_1, samplePos).r;
#else
        voxel = texture3D(volume_1, samplePos).r;
#endif


#ifdef MIP
#ifdef LOCAL_MIP
        if (voxel <= ch1V && ch1V >= local_MIP_threshold) {
          finished = true;
        } else if (voxel > ch1V) {
          ch1V = voxel;
          rayDepth = currentRayLength;
        }
#else
        if (voxel > ch1V) {
          ch1V = voxel;
          rayDepth = currentRayLength;
        }
        finished = ch1V >= 1.0;
#endif
#else
#if GLSL_VERSION >= 130
        vec4 color = texture(transfer_function_1, voxel);
#else
        vec4 color = texture1D(transfer_function_1, voxel);
#endif

        if (color.a > 0.0) {
          color.a / sampling_rate;
          result = COMPOSITING(result, color, currentRayLength, rayDepth);
          if (result.a >= 1.0) {
            result.a = 1.0;
            finished = true;
          }
        }
#endif // MIP

        currentRayLength += stepSize;
        finished = finished || (currentRayLength > 1.0);
      }
    }

#ifdef MIP
#if GLSL_VERSION >= 130
    result = texture(transfer_function_1, ch1V);
#else
    result = texture1D(transfer_function_1, ch1V);
#endif
#endif // MIP

#ifdef RESULT_OPAQUE
    result.a = 1.0;
#endif

    if (rayDepth >= 0.0) {
      //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
      // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
#if GLSL_VERSION >= 130
      float zeFront = texelFetch(ray_entry_eye_coord, ivec2(gl_FragCoord.xy), 0).z;
      float zeBack = texelFetch(ray_exit_eye_coord, ivec2(gl_FragCoord.xy), 0).z;
#else
      float zeFront = texture2D(ray_entry_eye_coord, texCoords).z;
      float zeBack = texture2D(ray_exit_eye_coord, texCoords).z;
#endif
      float ze = zeFront + rayDepth * (zeBack-zeFront);
      gl_FragDepth = ze_to_zw_a / ze + ze_to_zw_b;
    } else {
#ifdef RESULT_OPAQUE
      gl_FragDepth = entryTexCoordAndZ.w;
#else
      gl_FragDepth = 1.0;
#endif
    }

    result.rgb *= result.a;
    FragData0 = result;
  }
}

