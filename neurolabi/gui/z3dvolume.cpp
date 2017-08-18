/*
 * Copyright (C) 2005-2012 University of Muenster, Germany.
 * Visualization and Computer Graphics Group <http://viscg.uni-muenster.de>
 * For a list of authors please refer to the file "CREDITS.txt".
 * Copyright (C) 2012-2013 Korea Institiute of Science and Technologhy, Seoul.
 * Linqing Feng, Jinny Kim's lab <http://jinny.kist.re.kr>
 *
 * This file is derived from code of the free Voreen software package.
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2 as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License in the file
 * "LICENSE.txt" along with this file. If not, see <http://www.gnu.org/licenses/>.
 */

#include "z3dvolume.h"
#include <algorithm>
#include <QString>
#include "QsLog.h"
#include "z3dshaderprogram.h"
#include "z3dgpuinfo.h"
#include "z3dtexture.h"

Z3DVolume::Z3DVolume(Stack *stack, const glm::vec3 &spacing,
                     const glm::vec3 &offset, const glm::mat4 &transformation, QObject *parent)
  : QObject(parent)
  , m_stack(stack)
  , m_histogramMaxValue(-1)
  , m_volColor(1.f,1.f,1.f)
{
  m_dimensions = glm::uvec3(m_stack->width, m_stack->height, m_stack->depth);
  m_detailVolumeDimensions = glm::round(glm::vec3(m_dimensions) * spacing);
  m_parentVolumeDimensions = m_detailVolumeDimensions;
  m_parentVolumeOffset = offset;
  m_data.array = m_stack->array;
  setSpacing(spacing);
  setOffset(offset);
  setPhysicalToWorldMatrix(transformation);
  computeMaxValue();
  computeMinValue();
}

Z3DVolume::~Z3DVolume()
{
  if (m_histogramThread) {
    if (m_histogramThread->isRunning()) {
      m_histogramThread->wait();
    }
  }
  C_Stack::kill(m_stack);
}

int Z3DVolume::bitsStored() const
{
  if (m_stack->kind == GREY)
    return 8;
  else if (m_stack->kind == GREY16) {
    if (maxValue() <= 4096)
      return 12;
    else
      return 16;
  } else if (m_stack->kind == FLOAT32)
    return 32;
  return 0;
}

size_t Z3DVolume::numVoxels() const
{
  if (m_stack == NULL) {
    return 0;
  }

  return C_Stack::voxelNumber(m_stack);
//  return (size_t)m_stack->depth * (size_t)m_stack->height * (size_t)m_stack->width;
}

QString Z3DVolume::samplerType() const
{
  if (m_dimensions.z > 1)
    return "sampler3D";
  else if (m_dimensions.y > 1 && m_dimensions.x > 1)
    return "sampler2D";
  else
    return "sampler1D";
}

double Z3DVolume::floatMinValue() const
{
  if (bitsStored() <= 16)
    return m_minValue / ((1 << bitsStored()) - 1);
  else
    return minValue();   // already float image
}

double Z3DVolume::floatMaxValue() const
{
  if (bitsStored() <= 16)
    return m_maxValue / ((1 << bitsStored()) - 1);
  else
    return maxValue();   // already float image
}

double Z3DVolume::value(int x, int y, int z) const
{
  size_t stride_x = (size_t)m_stack->kind;
  size_t stride_y = (size_t)m_stack->kind * (size_t)m_stack->width;
  size_t stride_z = (size_t)m_stack->kind * (size_t)m_stack->width * (size_t)m_stack->height;
  if (m_stack->kind == GREY) {
    return *(m_data.array8 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (m_stack->kind == GREY16) {
    return *(m_data.array16 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  } else if (m_stack->kind == FLOAT32) {
    return *(m_data.array32 + (size_t)z*stride_z + (size_t)y*stride_y + (size_t)x*stride_x);
  }
  return 0.0;
}

double Z3DVolume::value(size_t index) const
{
  if (m_stack->kind == GREY) {
    return *(m_data.array8 + index);
  } else if (m_stack->kind == GREY16) {
    return *(m_data.array16 + index);
  } else if (m_stack->kind == FLOAT32) {
    return *(m_data.array32 + index);
  }
  return 0.0;
}

void Z3DVolume::asyncGenerateHistogram()
{
  if (hasHistogram() || m_histogramThread)
    return;
  m_histogramThread.reset(new Z3DVolumeHistogramThread(this));
  connect(m_histogramThread.get(), &Z3DVolumeHistogramThread::finished, this, &Z3DVolume::setHistogram);
  m_histogramThread->start();
}

size_t Z3DVolume::histogramBinCount() const
{
  if (m_stack->kind == GREY) {
    return 256;
  } else if (m_stack->kind == GREY16) {
    return bitsStored() == 16 ? 65536 : 4096;
  } else if (m_stack->kind == FLOAT32) {
    return m_histogram.size();
  }
  return 0;
}

size_t Z3DVolume::histogramValue(size_t index) const
{
  if (index < m_histogram.size())
    return m_histogram[index];
  else
    return 0;
}

size_t Z3DVolume::histogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(histogramBinCount()-1));
  return histogramValue(index);
}

double Z3DVolume::normalizedHistogramValue(size_t index) const
{
  return static_cast<double>(histogramValue(index)) / m_histogramMaxValue;
}

double Z3DVolume::normalizedHistogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(histogramBinCount()-1));
  return normalizedHistogramValue(index);
}

double Z3DVolume::logNormalizedHistogramValue(size_t index) const
{
  return std::log(static_cast<double>(histogramValue(index)) + 1.0) / std::log(m_histogramMaxValue + 1.0);
}

double Z3DVolume::logNormalizedHistogramValue(double fraction) const
{
  size_t index = static_cast<size_t>(fraction * static_cast<double>(histogramBinCount() - 1));
  return logNormalizedHistogramValue(index);
}

Z3DTexture* Z3DVolume::texture() const
{
  if (!m_texture) {
    generateTexture();
  }
  return m_texture.get();
}

ZBBox<glm::dvec3> Z3DVolume::worldBoundBox() const
{
  if (m_hasTransformMatrix) {
    ZBBox<glm::dvec3> res;
    res.expand(glm::dvec3(worldLUF()));
    res.expand(glm::dvec3(worldLDB()));
    res.expand(glm::dvec3(worldLDF()));
    res.expand(glm::dvec3(worldLUB()));
    res.expand(glm::dvec3(worldRUF()));
    res.expand(glm::dvec3(worldRDB()));
    res.expand(glm::dvec3(worldRDF()));
    res.expand(glm::dvec3(worldRUB()));

    return res;
  } else {
    return physicalBoundBox();
  }
}

void Z3DVolume::setPhysicalToWorldMatrix(const glm::mat4 &transformationMatrix)
{
  m_transformationMatrix = transformationMatrix;
  if (m_transformationMatrix != glm::mat4(1.0))
    m_hasTransformMatrix = true;
}

glm::mat4 Z3DVolume::voxelToWorldMatrix() const
{
  return physicalToWorldMatrix() * voxelToPhysicalMatrix();
}

glm::mat4 Z3DVolume::worldToVoxelMatrix() const
{
  return glm::inverse(voxelToWorldMatrix());
}

glm::mat4 Z3DVolume::worldToTextureMatrix() const
{
  return glm::inverse(textureToWorldMatrix());
}

glm::mat4 Z3DVolume::textureToWorldMatrix() const
{
  return voxelToWorldMatrix() * textureToVoxelMatrix();
}

glm::mat4 Z3DVolume::voxelToPhysicalMatrix() const
{
  // 1. Multiply by spacing 2. Apply offset
  glm::mat4 scale = glm::scale(glm::mat4(1.0), spacing());
  return glm::translate(scale, offset());
}

glm::mat4 Z3DVolume::physicalToVoxelMatrix() const
{
  glm::mat4 translate = glm::translate(glm::mat4(1.0), -offset());
  return glm::scale(translate, 1.f / spacing());
}

glm::mat4 Z3DVolume::worldToPhysicalMatrix() const
{
  return glm::inverse(physicalToWorldMatrix());
}

glm::mat4 Z3DVolume::textureToPhysicalMatrix() const
{
  return voxelToPhysicalMatrix() * textureToVoxelMatrix();
}

glm::mat4 Z3DVolume::physicalToTextureMatrix() const
{
  return voxelToTextureMatrix() * physicalToVoxelMatrix();
}

glm::mat4 Z3DVolume::textureToVoxelMatrix() const
{
  return glm::scale(glm::mat4(1.0), glm::vec3(dimensions()));
}

glm::mat4 Z3DVolume::voxelToTextureMatrix() const
{
  return glm::scale(glm::mat4(1.0), 1.0f / glm::vec3(dimensions()));
}

void Z3DVolume::setHistogram()
{
  m_histogram.swap(m_histogramThread->histogram());
  computeHistogramMaxValue();
  emit histogramFinished();
}

void Z3DVolume::generateTexture() const
{
  if (dimensions().x == 0 || dimensions().y == 0 || dimensions().z == 0) {
    QString message = QString("OpenGL volumes must have a size greater than 0 in all dimensions. Actual size: (%1, %2, %3)")
        .arg(m_stack->width).arg(m_stack->height).arg(m_stack->depth);
    LERROR() << message;
    return;
  }

  GLenum format;
  GLint internalFormat;
  GLenum dataType;
  if (m_stack->kind == GREY) {
    format = GL_RED;
    internalFormat = GLint(GL_R8);
    dataType = GL_UNSIGNED_BYTE;
  } else if (m_stack->kind == GREY16) {
    format = GL_RED;
    internalFormat = GLint(GL_R16);
    dataType = GL_UNSIGNED_SHORT;
  } else if (m_stack->kind == FLOAT32) {
    format = GL_RED;
    internalFormat = GLint(GL_R32F);
    dataType = GL_FLOAT;
  } else {
    LERROR() << "Only GREY, GREY16 or FLOAT32 stack formats are supported";
    return;
  }

  // Create texture
  m_texture.reset(new Z3DTexture(internalFormat, dimensions(), format, dataType));
  m_texture->uploadImage(m_stack->array);
}

void Z3DVolume::computeMinValue()
{
  if (m_stack->kind == GREY)
    m_minValue = *std::min_element(m_data.array8, m_data.array8 + numVoxels());
  else if (m_stack->kind == GREY16)
    m_minValue = *std::min_element(m_data.array16, m_data.array16 + numVoxels());
  else if (m_stack->kind == FLOAT32)
    m_minValue = *std::min_element(m_data.array32, m_data.array32 + numVoxels());
}

void Z3DVolume::computeMaxValue()
{
  if (m_stack->kind == GREY)
    m_maxValue = *std::max_element(m_data.array8, m_data.array8 + numVoxels());
  else if (m_stack->kind == GREY16)
    m_maxValue = *std::max_element(m_data.array16, m_data.array16 + numVoxels());
  else if (m_stack->kind == FLOAT32)
    m_maxValue = *std::max_element(m_data.array32, m_data.array32 + numVoxels());
}

void Z3DVolume::computeHistogramMaxValue()
{
  m_histogramMaxValue = *std::max_element(m_histogram.begin(), m_histogram.end());
}

//-----------------------------------------------------------------------------------
Z3DVolumeHistogramThread::Z3DVolumeHistogramThread(Z3DVolume *volume, QObject* parent)
  : QThread(parent)
  , m_volume(volume)
{
}

void Z3DVolumeHistogramThread::run()
{
  if (m_volume->getType() == GREY || m_volume->getType() == GREY16) {
    m_histogram.assign(static_cast<size_t>(m_volume->maxValue() + 1), 0);
    for (size_t i = 0; i < m_volume->numVoxels(); ++i) {
      m_histogram[static_cast<size_t>(m_volume->value(i))]++;
    }
  } else if (m_volume->getType() == FLOAT32) {
    size_t binCount = 256;   // cover range [0.0f, 1.0f]
    m_histogram.assign(binCount, 0);
    for (size_t i = 0; i < m_volume->numVoxels(); ++i) {
      m_histogram[static_cast<size_t>(m_volume->value(i) * (binCount-1))]++;
    }
  }
}

void Z3DVolume::translate(double dx, double dy, double dz)
{
  m_transformationMatrix[3][0] += dx;
  m_transformationMatrix[3][1] += dy;
  m_transformationMatrix[3][2] += dz;

  if (m_transformationMatrix != glm::mat4(1.0))
    m_hasTransformMatrix = true;
}
