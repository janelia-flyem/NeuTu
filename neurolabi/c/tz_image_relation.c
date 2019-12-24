/* tz_image_relation.c
 *
 * 12-Dec-2007 Initial write: Ting Zhao
 */

#include "tz_image_attribute.h"
#include "tz_image_relation.h"

_BOOL_ Image_Same_Size(const Image *image1, const Image *image2)
{
  if ((image1->width == image2->width) && (image1->height == image2->height)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Image_Same_Kind(const Image *image1, const Image *image2)
{
  if (Image_Kind(image1) == Image_Kind(image2)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Image_Same_Attribute(const Image *image1, const Image *image2)
{
  if ((image1 == NULL) && (image2 == NULL)) {
    return _TRUE_;
  }

  if ((Image_Same_Kind(image1, image2) == _TRUE_) && 
      (Image_Same_Size(image1, image2)==_TRUE_)) {
    return _TRUE_;
  } else {
    return _FALSE_;
  }
}

_BOOL_ Image_Identical(const Image *image1, const Image *image2)
{
  if (Image_Same_Size(image1, image2) && Image_Same_Kind(image1, image2)) {
    int n = Image_Array_Bsize(image1);
    int i;
    for (i = 0; i < n; i++) {
      if (image1->array[i] != image2->array[i]) {
	return _FALSE_;
      }
    }
  } else {
    return _FALSE_;
  }

  return _TRUE_;
}
