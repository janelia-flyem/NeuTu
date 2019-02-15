#ifndef ZRESOLUTIONTEST_H
#define ZRESOLUTIONTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zresolution.h"

#ifdef _USE_GTEST_

TEST(ZResolution, Basic)
{
  ZResolution resolution;
  resolution.setVoxelSize(1, 2, 3);
  resolution.setUnit(ZResolution::UNIT_PIXEL);
  ASSERT_DOUBLE_EQ(1.0, resolution.voxelSizeX());
  ASSERT_DOUBLE_EQ(2.0, resolution.voxelSizeY());
  ASSERT_DOUBLE_EQ(3.0, resolution.voxelSizeZ());
  ASSERT_EQ(ZResolution::UNIT_PIXEL, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_NANOMETER));

  resolution.setUnit(ZResolution::UNIT_MICRON);
  ASSERT_EQ(ZResolution::UNIT_MICRON, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1000.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2000.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3000.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_NANOMETER));

  resolution.setUnit(ZResolution::UNIT_NANOMETER);
  ASSERT_EQ(ZResolution::UNIT_NANOMETER, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        0.001, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.002, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.003, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::UNIT_NANOMETER));


  ASSERT_DOUBLE_EQ(
        2.0, resolution.getPlaneVoxelSize(neutu::EPlane::XY, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getPlaneVoxelSize(neutu::EPlane::XZ, ZResolution::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        6.0, resolution.getPlaneVoxelSize(neutu::EPlane::YZ, ZResolution::UNIT_PIXEL));
  /*
  ASSERT_DOUBLE_EQ(
        0.001, resolution.getVoxelSize(neutube::X_AXIS, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.002, resolution.getVoxelSize(neutube::Y_AXIS, ZResolution::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.003, resolution.getVoxelSize(neutube::Z_AXIS, ZResolution::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutube::X_AXIS, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutube::Y_AXIS, ZResolution::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutube::Z_AXIS, ZResolution::UNIT_NANOMETER));
        */
}


#endif

#endif // ZRESOLUTIONTEST_H
