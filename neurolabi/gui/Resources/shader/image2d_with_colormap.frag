
#if NUM_VOLUMES >= 1
uniform sampler2D volume_1;
uniform sampler1D colormap_1;
#endif

#if NUM_VOLUMES >= 2
uniform sampler2D volume_2;
uniform sampler1D colormap_2;
#endif

#if NUM_VOLUMES >= 3
uniform sampler2D volume_3;
uniform sampler1D colormap_3;
#endif

#if NUM_VOLUMES >= 4
uniform sampler2D volume_4;
uniform sampler1D colormap_4;
#endif

#if NUM_VOLUMES >= 5
uniform sampler2D volume_5;
uniform sampler1D colormap_5;
#endif

#if NUM_VOLUMES >= 6
uniform sampler2D volume_6;
uniform sampler1D colormap_6;
#endif

#if NUM_VOLUMES >= 7
uniform sampler2D volume_7;
uniform sampler1D colormap_7;
#endif

#if NUM_VOLUMES >= 8
uniform sampler2D volume_8;
uniform sampler1D colormap_8;
#endif

#if NUM_VOLUMES >= 9
uniform sampler2D volume_9;
uniform sampler1D colormap_9;
#endif

#if NUM_VOLUMES >= 10
uniform sampler2D volume_10;
uniform sampler1D colormap_10;
#endif

#if NUM_VOLUMES >= 11
uniform sampler2D volume_11;
uniform sampler1D colormap_11;
#endif

#if NUM_VOLUMES >= 12
uniform sampler2D volume_12;
uniform sampler1D colormap_12;
#endif

#if NUM_VOLUMES >= 13
uniform sampler2D volume_13;
uniform sampler1D colormap_13;
#endif

#if NUM_VOLUMES >= 14
uniform sampler2D volume_14;
uniform sampler1D colormap_14;
#endif

#if NUM_VOLUMES >= 15
uniform sampler2D volume_15;
uniform sampler1D colormap_15;
#endif

#if NUM_VOLUMES >= 16
uniform sampler2D volume_16;
uniform sampler1D colormap_16;
#endif

#if NUM_VOLUMES >= 17
uniform sampler2D volume_17;
uniform sampler1D colormap_17;
#endif

#if NUM_VOLUMES >= 18
uniform sampler2D volume_18;
uniform sampler1D colormap_18;
#endif

#if NUM_VOLUMES >= 19
uniform sampler2D volume_19;
uniform sampler1D colormap_19;
#endif

#if NUM_VOLUMES >= 20
uniform sampler2D volume_20;
uniform sampler1D colormap_20;
#endif

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
  vec4 color = vec4(0.0);
  vec4 chColor;

#if NUM_VOLUMES >= 1
#if GLSL_VERSION >= 130
  chColor = texture(colormap_1, texture(volume_1, texCoord0).r);
#else
  chColor = texture1D(colormap_1, texture2D(volume_1, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 2
#if GLSL_VERSION >= 130
  chColor = texture(colormap_2, texture(volume_2, texCoord0).r);
#else
  chColor = texture1D(colormap_2, texture2D(volume_2, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 3
#if GLSL_VERSION >= 130
  chColor = texture(colormap_3, texture(volume_3, texCoord0).r);
#else
  chColor = texture1D(colormap_3, texture2D(volume_3, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 4
#if GLSL_VERSION >= 130
  chColor = texture(colormap_4, texture(volume_4, texCoord0).r);
#else
  chColor = texture1D(colormap_4, texture2D(volume_4, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 5
#if GLSL_VERSION >= 130
  chColor = texture(colormap_5, texture(volume_5, texCoord0).r);
#else
  chColor = texture1D(colormap_5, texture2D(volume_5, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 6
#if GLSL_VERSION >= 130
  chColor = texture(colormap_6, texture(volume_6, texCoord0).r);
#else
  chColor = texture1D(colormap_6, texture2D(volume_6, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 7
#if GLSL_VERSION >= 130
  chColor = texture(colormap_7, texture(volume_7, texCoord0).r);
#else
  chColor = texture1D(colormap_7, texture2D(volume_7, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 8
#if GLSL_VERSION >= 130
  chColor = texture(colormap_8, texture(volume_8, texCoord0).r);
#else
  chColor = texture1D(colormap_8, texture2D(volume_8, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 9
#if GLSL_VERSION >= 130
  chColor = texture(colormap_9, texture(volume_9, texCoord0).r);
#else
  chColor = texture1D(colormap_9, texture2D(volume_9, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 10
#if GLSL_VERSION >= 130
  chColor = texture(colormap_10, texture(volume_10, texCoord0).r);
#else
  chColor = texture1D(colormap_10, texture2D(volume_10, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 11
#if GLSL_VERSION >= 130
  chColor = texture(colormap_11, texture(volume_11, texCoord0).r);
#else
  chColor = texture1D(colormap_11, texture2D(volume_11, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 12
#if GLSL_VERSION >= 130
  chColor = texture(colormap_12, texture(volume_12, texCoord0).r);
#else
  chColor = texture1D(colormap_12, texture2D(volume_12, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 13
#if GLSL_VERSION >= 130
  chColor = texture(colormap_13, texture(volume_13, texCoord0).r);
#else
  chColor = texture1D(colormap_13, texture2D(volume_13, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 14
#if GLSL_VERSION >= 130
  chColor = texture(colormap_14, texture(volume_14, texCoord0).r);
#else
  chColor = texture1D(colormap_14, texture2D(volume_14, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 15
#if GLSL_VERSION >= 130
  chColor = texture(colormap_15, texture(volume_15, texCoord0).r);
#else
  chColor = texture1D(colormap_15, texture2D(volume_15, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 16
#if GLSL_VERSION >= 130
  chColor = texture(colormap_16, texture(volume_16, texCoord0).r);
#else
  chColor = texture1D(colormap_16, texture2D(volume_16, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 17
#if GLSL_VERSION >= 130
  chColor = texture(colormap_17, texture(volume_17, texCoord0).r);
#else
  chColor = texture1D(colormap_17, texture2D(volume_17, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 18
#if GLSL_VERSION >= 130
  chColor = texture(colormap_18, texture(volume_18, texCoord0).r);
#else
  chColor = texture1D(colormap_18, texture2D(volume_18, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 19
#if GLSL_VERSION >= 130
  chColor = texture(colormap_19, texture(volume_19, texCoord0).r);
#else
  chColor = texture1D(colormap_19, texture2D(volume_19, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

#if NUM_VOLUMES >= 20
#if GLSL_VERSION >= 130
  chColor = texture(colormap_20, texture(volume_20, texCoord0).r);
#else
  chColor = texture1D(colormap_20, texture2D(volume_20, texCoord0).r);
#endif
  color = max(color, chColor);
#endif

  color.rgb *= color.a;
  FragData0 = color;
#else
  discard;
#endif
}

