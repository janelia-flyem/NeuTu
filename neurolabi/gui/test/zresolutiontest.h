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
  resolution.setUnit(ZResolution::EUnit::UNIT_PIXEL);
  ASSERT_DOUBLE_EQ(1.0, resolution.voxelSizeX());
  ASSERT_DOUBLE_EQ(2.0, resolution.voxelSizeY());
  ASSERT_DOUBLE_EQ(3.0, resolution.voxelSizeZ());
  ASSERT_EQ(ZResolution::EUnit::UNIT_PIXEL, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_NANOMETER));

  resolution.setUnit(ZResolution::EUnit::UNIT_MICRON);
  ASSERT_EQ(ZResolution::EUnit::UNIT_MICRON, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1000.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2000.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3000.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_NANOMETER));

  resolution.setUnit(ZResolution::EUnit::UNIT_NANOMETER);
  ASSERT_EQ(ZResolution::EUnit::UNIT_NANOMETER, resolution.getUnit());

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_PIXEL));

  ASSERT_DOUBLE_EQ(
        0.001, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.002, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.003, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutu::EAxis::X, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutu::EAxis::Y, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutu::EAxis::Z, ZResolution::EUnit::UNIT_NANOMETER));


  ASSERT_DOUBLE_EQ(
        2.0, resolution.getPlaneVoxelSize(neutu::EPlane::XY, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getPlaneVoxelSize(neutu::EPlane::XZ, ZResolution::EUnit::UNIT_PIXEL));
  ASSERT_DOUBLE_EQ(
        6.0, resolution.getPlaneVoxelSize(neutu::EPlane::YZ, ZResolution::EUnit::UNIT_PIXEL));
  /*
  ASSERT_DOUBLE_EQ(
        0.001, resolution.getVoxelSize(neutube::X_AXIS, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.002, resolution.getVoxelSize(neutube::Y_AXIS, ZResolution::EUnit::UNIT_MICRON));
  ASSERT_DOUBLE_EQ(
        0.003, resolution.getVoxelSize(neutube::Z_AXIS, ZResolution::EUnit::UNIT_MICRON));

  ASSERT_DOUBLE_EQ(
        1.0, resolution.getVoxelSize(neutube::X_AXIS, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        2.0, resolution.getVoxelSize(neutube::Y_AXIS, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_DOUBLE_EQ(
        3.0, resolution.getVoxelSize(neutube::Z_AXIS, ZResolution::EUnit::UNIT_NANOMETER));
        */
}


#endif

#endif // ZRESOLUTIONTEST_H
