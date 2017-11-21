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

#ifndef Z3DVOLUME_H
#define Z3DVOLUME_H

#include "z3dgl.h"
#include "zstack.hxx"
#include "zbbox.h"
#include <QObject>
#include <QThread>

class Z3DVolume;

class Z3DTexture;

class Z3DVolumeHistogramThread : public QThread
{
public:
  explicit Z3DVolumeHistogramThread(Z3DVolume* volume, QObject* parent = nullptr);

  void run();

  std::vector<size_t>& histogram()
  { return m_histogram; }

private:
  Z3DVolume* m_volume;
  std::vector<size_t> m_histogram;
};

// Z3DVolume coordinates:
// 1. Voxel Coordinate:    [0, dim.x-1] x [0, dim.y-1] x [0, dim.z-1]
//                     in which (0,0,0) is LeftUpFront Corner (LUF)
//                         and dim-1 is RightDownBack Corner (RDB)
// 2. Texture Coordinate:  [0.0, 1.0] x [0.0, 1.0] x [0.0, 1.0]
// 2. Physical Coordinate: VoxelCoord * Spacing + Offset
//                 Note: Offset is physical space offset (after scaling)
// 3. World Coordinate: TransformationMatrix * PhysicalCoord

// Z3DVolume data source info:
// ParentVolume ------crop----->  DetailVolume -------downsample------>  current Z3DVolume

// spacing is usually caused by downsampling a volume to fit it into graphical memory

// only one channel GREY8, GREY16 or FLOAT32 volume is supported, so convert first
class Z3DVolume : public QObject
{
  Q_OBJECT
public:
  // Z3DVolume will take ownership of the stack
  Z3DVolume(Stack* stack,
            const glm::vec3& spacing = glm::vec3(1.f),
            const glm::vec3& offset = glm::vec3(0.f),
            const glm::mat4& transformation = glm::mat4(1.0),
            QObject *parent = nullptr);
  virtual ~Z3DVolume();

  // by default same as dimension.
  void setParentVolumeDimensions(const glm::uvec3& dim)
  { m_parentVolumeDimensions = dim; }

  // by default same as current offset
  void setParentVolumeOffset(const glm::vec3& of)
  { m_parentVolumeOffset = of; }

  bool isSubVolume() const
  { return m_parentVolumeDimensions != m_detailVolumeDimensions; }

  bool isDownsampledVolume() const
  { return m_detailVolumeDimensions != m_dimensions; }

  // actual data dimension of current stack
  glm::uvec3 dimensions() const
  { return m_dimensions; }

  // detail stack dimension, current stack might be downsampled from detail stack
  glm::uvec3 originalDimensions() const
  { return m_detailVolumeDimensions; }

  // parent stack dimension, detailed stack might be cropped form parent stack
  glm::uvec3 parentVolumeDimensions() const
  { return m_parentVolumeDimensions; }

  int bitsStored() const;

  size_t numVoxels() const;

  bool is1DData() const
  { return m_dimensions.z == 1 && (m_dimensions.x == 1 || m_dimensions.y == 1); }

  bool is2DData() const
  { return m_dimensions.z == 1 && m_dimensions.x > 1 && m_dimensions.y > 1; }

  bool is3DData() const
  { return m_dimensions.z > 1/* && m_dimensions.x > 1 && m_dimensions.y > 1*/; }

  // Returns a string representation of the sampler type: "sampler2D" for 2D image, "sampler3D" for 3D volume
  QString samplerType() const;

  glm::vec3 spacing() const
  { return m_spacing; }

  glm::vec3 offset() const
  { return m_offset; }

  double minValue() const
  { return m_minValue; }

  double maxValue() const
  { return m_maxValue; }

  // float version return pixel value in range [0.0 1.0]
  double floatMinValue() const;

  double floatMaxValue() const;

  double value(int x, int y, int z) const;

  double value(size_t index) const;

  // to use histogram, first check hasHistogram(), if not, then call asyncGenerateHistogram() and wait for signal histogramFinished().
  bool hasHistogram() const
  { return !m_histogram.empty(); }

  void asyncGenerateHistogram();

  size_t histogramBinCount() const;

  size_t histogramValue(size_t index) const;

  size_t histogramValue(double fraction) const;    // input value in range [0.0, 1.0]
  double normalizedHistogramValue(size_t index) const;

  double normalizedHistogramValue(double fraction) const;

  double logNormalizedHistogramValue(size_t index) const;

  double logNormalizedHistogramValue(double fraction) const;

  Z3DTexture* texture() const;

  int getType() const { return m_stack->kind; }

  // Useful coordinate L->Left U->Up F->Front R->Right D->Down B->Back

  glm::vec3 cubeSize() const
  { return glm::vec3(dimensions()) * spacing(); }

  glm::vec3 physicalLUF() const
  { return offset(); }

  glm::vec3 physicalRDB() const
  { return offset() + glm::max(glm::vec3(1.f), glm::vec3(dimensions()) - 1.f) * spacing(); }

  glm::vec3 physicalLDF() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRDF() const
  { return glm::vec3(physicalRDB().x, physicalRDB().y, physicalLUF().z); }

  glm::vec3 physicalRUF() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalLUF().z); }

  glm::vec3 physicalLUB() const
  { return glm::vec3(physicalLUF().x, physicalLUF().y, physicalRDB().z); }

  glm::vec3 physicalLDB() const
  { return glm::vec3(physicalLUF().x, physicalRDB().y, physicalRDB().z); }

  glm::vec3 physicalRUB() const
  { return glm::vec3(physicalRDB().x, physicalLUF().y, physicalRDB().z); }

  ZBBox<glm::dvec3> physicalBoundBox() const
  { return ZBBox<glm::dvec3>(glm::dvec3(physicalLUF()), glm::dvec3(physicalRDB())); }

  // bound voxels in world coordinate
  glm::vec3 worldLUF() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalLUF()) : physicalLUF(); }

  glm::vec3 worldRDB() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalRDB()) : physicalRDB(); }

  glm::vec3 worldLDF() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalLDF()) : physicalLDF(); }

  glm::vec3 worldRDF() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalRDF()) : physicalRDF(); }

  glm::vec3 worldRUF() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalRUF()) : physicalRUF(); }

  glm::vec3 worldLUB() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalLUB()) : physicalLUB(); }

  glm::vec3 worldLDB() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalLDB()) : physicalLDB(); }

  glm::vec3 worldRUB() const
  { return m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, physicalRUB()) : physicalRUB(); }

  ZBBox<glm::dvec3> worldBoundBox() const;

  // corners of parent volume
  glm::vec3 parentVolPhysicalLUF() const
  { return m_parentVolumeOffset; }

  glm::vec3 parentVolPhysicalRDB() const
  { return m_parentVolumeOffset + glm::max(glm::vec3(1.f), (glm::vec3(m_parentVolumeDimensions) - 1.f)); }

  glm::vec3 parentVolPhysicalLDF() const
  { return glm::vec3(parentVolPhysicalLUF().x, parentVolPhysicalRDB().y, parentVolPhysicalLUF().z); }

  glm::vec3 parentVolPhysicalRDF() const
  { return glm::vec3(parentVolPhysicalRDB().x, parentVolPhysicalRDB().y, parentVolPhysicalLUF().z); }

  glm::vec3 parentVolPhysicalRUF() const
  { return glm::vec3(parentVolPhysicalRDB().x, parentVolPhysicalLUF().y, parentVolPhysicalLUF().z); }

  glm::vec3 parentVolPhysicalLUB() const
  { return glm::vec3(parentVolPhysicalLUF().x, parentVolPhysicalLUF().y, parentVolPhysicalRDB().z); }

  glm::vec3 parentVolPhysicalLDB() const
  { return glm::vec3(parentVolPhysicalLUF().x, parentVolPhysicalRDB().y, parentVolPhysicalRDB().z); }

  glm::vec3 parentVolPhysicalRUB() const
  { return glm::vec3(parentVolPhysicalRDB().x, parentVolPhysicalLUF().y, parentVolPhysicalRDB().z); }

  glm::vec3 parentVolWorldLUF() const
  {
    return !isSubVolume() ? worldLUF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalLUF())
                                                 : parentVolPhysicalLUF();
  }

  glm::vec3 parentVolWorldRDB() const
  {
    return !isSubVolume() ? worldRDB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalRDB())
                                                 : parentVolPhysicalRDB();
  }

  glm::vec3 parentVolWorldLDF() const
  {
    return !isSubVolume() ? worldLDF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalLDF())
                                                 : parentVolPhysicalLDF();
  }

  glm::vec3 parentVolWorldRDF() const
  {
    return !isSubVolume() ? worldRDF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalRDF())
                                                 : parentVolPhysicalRDF();
  }

  glm::vec3 parentVolWorldRUF() const
  {
    return !isSubVolume() ? worldRUF() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalRUF())
                                                 : parentVolPhysicalRUF();
  }

  glm::vec3 parentVolWorldLUB() const
  {
    return !isSubVolume() ? worldLUB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalLUB())
                                                 : parentVolPhysicalLUB();
  }

  glm::vec3 parentVolWorldLDB() const
  {
    return !isSubVolume() ? worldLDB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalLDB())
                                                 : parentVolPhysicalLDB();
  }

  glm::vec3 parentVolWorldRUB() const
  {
    return !isSubVolume() ? worldRUB() :
                            m_hasTransformMatrix ? glm::applyMatrix(m_transformationMatrix, parentVolPhysicalRUB())
                                                 : parentVolPhysicalRUB();
  }

  void setSpacing(const glm::vec3& spacing)
  { m_spacing = spacing; }

  void setOffset(const glm::vec3& offset)
  { m_offset = offset; }

  void setPhysicalToWorldMatrix(const glm::mat4& transformationMatrix);

  void translate(double dx, double dy, double dz);

  // color of this channel, max intensity will map to this color
  void setVolColor(const glm::vec3& col)
  { m_volColor = glm::clamp(col, glm::vec3(0.f), glm::vec3(1.f)); }

  glm::vec3 volColor() const
  { return m_volColor; }

  // Returns the matrix mapping from voxel coordinates (i.e. [0; dim-1]) to world coordinates.
  glm::mat4 voxelToWorldMatrix() const;

  // Returns the matrix mapping from world coordinates to voxel coordinates (i.e. [0; dim-1]).
  glm::mat4 worldToVoxelMatrix() const;

  // Returns the matrix mapping from world coordinates to texture coordinates (i.e. [0.0; 1.0]).
  glm::mat4 worldToTextureMatrix() const;

  // Returns the matrix mapping from texture coordinates (i.e. [0.0; 1.0]) to world coordinates.
  glm::mat4 textureToWorldMatrix() const;

  glm::mat4 voxelToPhysicalMatrix() const;

  glm::mat4 physicalToVoxelMatrix() const;

  glm::mat4 physicalToWorldMatrix() const
  { return m_transformationMatrix; }

  glm::mat4 worldToPhysicalMatrix() const;

  glm::mat4 textureToPhysicalMatrix() const;

  glm::mat4 physicalToTextureMatrix() const;

  glm::mat4 textureToVoxelMatrix() const;

  glm::mat4 voxelToTextureMatrix() const;

signals:
  void histogramFinished();

protected:
  void setHistogram();

  void generateTexture() const;

private:
  void computeMinValue();
  void computeMaxValue();
  void computeHistogramMaxValue();

protected:
  Stack *m_stack;
  Image_Array m_data;
  glm::uvec3 m_dimensions;
  glm::uvec3 m_parentVolumeDimensions;
  glm::vec3 m_parentVolumeOffset;
  glm::uvec3 m_detailVolumeDimensions;
  double m_minValue;
  double m_maxValue;
  glm::vec3 m_spacing;
  glm::vec3 m_offset;
  glm::mat4 m_transformationMatrix;
  std::vector<size_t> m_histogram;
  double m_histogramMaxValue;
  mutable std::unique_ptr<Z3DTexture> m_texture;

  glm::vec3 m_volColor;

private:
  std::unique_ptr<Z3DVolumeHistogramThread> m_histogramThread;
  bool m_hasTransformMatrix;
};

#endif // Z3DVOLUME_H
