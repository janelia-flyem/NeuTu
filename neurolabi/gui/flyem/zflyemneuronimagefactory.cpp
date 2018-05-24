#include "zflyemneuronimagefactory.h"
#include "zobject3dscan.h"
#include "misc/miscutility.h"

ZFlyEmNeuronImageFactory::ZFlyEmNeuronImageFactory()
{
  for (int i = 0; i < 3; ++i) {
    m_sizePolicy[i] = SIZE_BOUND_BOX;
    m_sourceDimension[i] = 0;
    m_downsampleInterval[i] = 0;
  }
}

void ZFlyEmNeuronImageFactory::setSizePolicy(
    ESizePolicy xPolicy, ESizePolicy yPolicy, ESizePolicy zPolicy)
{
  m_sizePolicy[0] = xPolicy;
  m_sizePolicy[1] = yPolicy;
  m_sizePolicy[2] = zPolicy;
}

void ZFlyEmNeuronImageFactory::setSizePolicy(
    neutube::EAxis axis, ESizePolicy policy)
{
  switch (axis) {
  case neutube::X_AXIS:
    m_sizePolicy[0] = policy;
    break;
  case neutube::Y_AXIS:
    m_sizePolicy[1] = policy;
    break;
  case neutube::Z_AXIS:
    m_sizePolicy[2] = policy;
    break;
  case neutube::A_AXIS:
    break;
  }
}

int ZFlyEmNeuronImageFactory::getSourceDimension(neutube::EAxis axis) const
{
  switch (axis) {
  case neutube::X_AXIS:
    return m_sourceDimension[0];
  case neutube::Y_AXIS:
    return m_sourceDimension[1];
  case neutube::Z_AXIS:
    return m_sourceDimension[2];
  case neutube::A_AXIS:
    break;
  }

  return 0;
}

void ZFlyEmNeuronImageFactory::setDownsampleInterval(int dx, int dy, int dz)
{
  m_downsampleInterval[0] = dx;
  m_downsampleInterval[1] = dy;
  m_downsampleInterval[2] = dz;
}

void ZFlyEmNeuronImageFactory::setSourceDimension(
    int width, int height, int depth)
{
  m_sourceDimension[0] = width;
  m_sourceDimension[1] = height;
  m_sourceDimension[2] = depth;
}

Stack* ZFlyEmNeuronImageFactory::createImage(const ZObject3dScan &obj) const
{
  Stack *stack = NULL;
  if (!obj.isEmpty()) {
    ZObject3dScan objProj = obj.makeYProjection();
    objProj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[2], 0);
    Cuboid_I boundBox;
    objProj.getBoundBox(&boundBox);

    int width = 0;
    int height = 0;
    //int depth = 0;
    if (m_sizePolicy[0] == SIZE_SOURCE) {
      width = m_sourceDimension[0] / (m_downsampleInterval[0] + 1) + 1;
    }
    if (m_sizePolicy[2] == SIZE_SOURCE) {
      height = m_sourceDimension[2];

      if (m_downsampleInterval[2] > 0) {
        height /= m_downsampleInterval[2] + 1;
        height += 1;
      }
    }
    /*
    if (m_sizePolicy[2] == SIZE_SOURCE) {
      depth = m_sourceDimension[2];
    }
    */

    int offset[3] = { 0, 0, 0 };
    if (width == 0) {
      width = Cuboid_I_Width(&boundBox);
      offset[0] = -boundBox.cb[0];
    }
    if (height == 0) {
      height = Cuboid_I_Height(&boundBox);
      offset[1] = -boundBox.cb[1];
    }
    /*
    if (depth == 0) {
      depth = Cuboid_I_Depth(&boundBox);
      offset[2] = -boundBox.cb[2];
    }
    */

    if (width > 0 && height > 0/* && depth > 0*/) {
      stack = C_Stack::make(GREY, width, height, 1);
      C_Stack::setZero(stack);
      objProj.drawStack(stack, 255, offset);
    }
  }

  return stack;
}

Stack* ZFlyEmNeuronImageFactory::createImage(const ZFlyEmNeuron &neuron) const
{
  ZObject3dScan *obj = neuron.getBody();
  Stack *stack = NULL;
  if (obj != NULL) {
    stack = createSurfaceImage(*obj);
  }

  return stack;
}

Stack *ZFlyEmNeuronImageFactory::createSurfaceImage(const ZObject3dScan &obj) const
{
  Stack *stack = NULL;
  if (!obj.isEmpty()) {
    ZObject3dScan tmpObj = obj;
    tmpObj.downsampleMax(m_downsampleInterval[0], m_downsampleInterval[1],
        m_downsampleInterval[2]);

    int offset[3] = { 0, 0, 0 };
//    tmpObj.switchYZ();
    Stack *objStack = tmpObj.toStack(offset);
    offset[2] -= m_sourceDimension[2] / (m_downsampleInterval[2] + 1);

#ifdef _DEBUG_2
    C_Stack::write("/Users/zhaot/Work/neutube/neurolabi/data/test2.tif", objStack);
#endif

    stack = misc::computeNormal(objStack, neutube::Y_AXIS);
    C_Stack::kill(objStack);

    int height= C_Stack::height(stack);
    if (m_sizePolicy[2] == SIZE_SOURCE) {
      height = m_sourceDimension[2];

      if (m_downsampleInterval[2] > 0) {
        height /= m_downsampleInterval[2] + 1;
        height += 1;
      }
    }
    if (height > C_Stack::height(stack)) {
      Stack *out = C_Stack::crop(stack, 0, -offset[1], 0, C_Stack::width(stack),
          height, 1, NULL);
      C_Stack::kill(stack);
      stack = out;
    }
  }

  return stack;
}
