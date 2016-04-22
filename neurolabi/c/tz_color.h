/**@file tz_color.h
 * @brief RGB color
 * @author Ting Zhao
 * @date 24-Sep-2008
 */

#ifndef _TZ_COLOR_H_
#define _TZ_COLOR_H_

#include "tz_stdint.h"
#include "tz_cdefs.h"

__BEGIN_DECLS

const static uint8_t Jet_Colormap[192] = { 
  0,  0,143,
  0,  0,159,
  0,  0,175,
  0,  0,191,
  0,  0,207,
  0,  0,223,
  0,  0,239,
  0,  0,255,
  0, 16,255,
  0, 32,255,
  0, 48,255,
  0, 64,255,
  0, 80,255,
  0, 96,255,
  0,112,255,
  0,128,255,
  0,143,255,
  0,159,255,
  0,175,255,
  0,191,255,
  0,207,255,
  0,223,255,
  0,239,255,
  0,255,255,
 16,255,239,
 32,255,223,
 48,255,207,
 64,255,191,
 80,255,175,
 96,255,159,
112,255,143,
128,255,128,
143,255,112,
159,255, 96,
175,255, 80,
191,255, 64,
207,255, 48,
223,255, 32,
239,255, 16,
255,255,  0,
255,239,  0,
255,223,  0,
255,207,  0,
255,191,  0,
255,175,  0,
255,159,  0,
255,143,  0,
255,128,  0,
255,112,  0,
255, 96,  0,
255, 80,  0,
255, 64,  0,
255, 48,  0,
255, 32,  0,
255, 16,  0,
255,  0,  0,
239,  0,  0,
223,  0,  0,
207,  0,  0,
191,  0,  0,
175,  0,  0,
159,  0,  0,
143,  0,  0,
128,  0,  0
};

const static int Jet_Color_Number = sizeof(Jet_Colormap) / 3;

const static uint8_t Discrete_Colormap[204] = { 
   160,0,0, 0,16,255, 112,255,144, 255,96,0, 0,32,255, 
   0,64,255, 255,80,0, 0,48,255,176,0,0, 0,0,176,  
  10,0,192, 28,0,0, 0,0,208, 0,224,255, 144,0,0,
  0,240,255, 255,32,0, 0,0,224, 240,0,0, 0,0,255,
  96,160,160, 224,0,0, 0,0,144, 128,255,128, 255,224,0,
  255,208,0, 0,96,0, 255,192,0, 208,255,48, 16,255,240, 
  240,255,16, 0,80,255, 160,255,96, 0,255,0,  224,255,32, 
  255,96,255, 0,255,224, 208,0,0, 48,255,208, 0,112,255, 
  64,255,192, 255,112,0, 0,128,255, 255,64,0, 192,0,0, 
  0,160,0, 0,144,255, 0,160,255, 192,255,64, 255,176,0, 
  0,176,255, 255,160,0, 0,192,255, 255,144,0, 255,255,0,  
  0,208,255, 255,128,0, 255,0,0, 80,255,176, 176,255,80, 
  255,48,0, 144,255,112, 0,255,255, 255,16,0, 0,0,160, 
  255,240,0, 0,0,240, 0,48,0
};

const static int Discrete_Color_Number = sizeof(Discrete_Colormap) / 3;

typedef struct _Rgb_Color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Rgb_Color;

Rgb_Color* New_Rgb_Color();
void Delete_Rgb_Color(Rgb_Color *color);

Rgb_Color* Make_Rgb_Color(uint8_t r, uint8_t g, uint8_t b);

/* 
 * Set_Color() sets RGB color for a color object. 
 * Set_Color_Hsv() sets RGB color from a point in the HSV space. Here h has
 * range [0, 6], s and v has range[0, 1].
 */

/**@brief Set RGB color.
 *
 * Set_Color() sets <color> to (<r>, <g>, <b>) with the RGB alignment.
 */
void Set_Color(Rgb_Color *color, uint8_t r, uint8_t g, uint8_t b);

/**@brief Set RGB color from an integer value
 *
 * r (0-7); g (8-15); b(16-23); (24-) ignored
 */
void Set_Color_From_Int(Rgb_Color *color, int c);

/**@brief Set RGB color form HSV.
 *
 * Set_Color() sets <color> from the HSV space.
 */
void Set_Color_Hsv(Rgb_Color *color, double h, double s, double v);

/**@brief Set RGB color from the Jet map.
 * 
 * The normal range of <index> is [0, 63]. Any value out of range will be mapped
 * as its mod to 64.
 */
void Set_Color_Jet(Rgb_Color *color, int index);


/**@brief Set RGB color from the Discrete map.
 */
void Set_Color_Discrete(Rgb_Color *color, int index);

void Print_Rgb_Color(const Rgb_Color *color);

/**@brief Print RGB color.
 *
 * Rgb_Color_Fprintln() prints <color> to <fp> with a new line at the end.
 */
void Rgb_Color_Fprintln(const Rgb_Color *color, FILE *fp);

void Rgb_Color_To_Hsv(const Rgb_Color *color, double *h, double *s, double *v);
double Rgb_Color_Hue(const Rgb_Color *color);

double Rgb_Color_Hue_Diff(const Rgb_Color *color1, const Rgb_Color *color2);

typedef struct _Tiff_Colormap {
  int length;
  uint16_t *array;
} Tiff_Colormap;

Tiff_Colormap* New_Tiff_Colormap();
void Delete_Tiff_Colormap(Tiff_Colormap *cmap);


__END_DECLS

#endif
