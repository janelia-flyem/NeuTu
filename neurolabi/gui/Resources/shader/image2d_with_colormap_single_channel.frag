uniform sampler2D volume_1;
uniform sampler1D colormap_1;

#if GLSL_VERSION >= 130
in vec2 texCoord0;
#else
varying vec2 texCoord0;
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
  FragData0 = texture(colormap_1, texture(volume_1, texCoord0).r);
#else
  FragData0 = texture1D(colormap_1, texture2D(volume_1, texCoord0).r);
#endif

  FragData0.rgb *= FragData0.a;
#else
  discard;
#endif
}

