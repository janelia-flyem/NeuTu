uniform sampler3D volume_1;
uniform sampler1D transfer_function_1;

#if GLSL_VERSION >= 130
in vec3 texCoord0;
#else
varying vec3 texCoord0;
#endif

#if GLSL_VERSION >= 330
layout(location = 0) out vec4 FragData0;
#elif GLSL_VERSION >= 130
out vec4 FragData0;  // call glBindFragDataLocation before linking
#else
#define FragData0 gl_FragData[0]
#endif

void main()
{
#if NUM_VOLUMES > 0

#if GLSL_VERSION >= 130
  vec4 color = texture(transfer_function_1, texture(volume_1, texCoord0).r);
#else
  vec4 color = texture1D(transfer_function_1, texture3D(volume_1, texCoord0).r);
#endif
  if (color.a == 0.0) {
    color = vec4(0.0);
  }

#ifdef RESULT_OPAQUE
  color.a = 1.0;
#else
  color.rgb *= color.a;
#endif

  FragData0 = color;
#else
  discard;
#endif
}

