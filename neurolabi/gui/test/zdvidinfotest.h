#ifndef ZDVIDINFOTEST_H
#define ZDVIDINFOTEST_H

#include "ztestheader.h"

#ifdef _USE_GTEST_

#include "geometry/zintcuboid.h"
#include "dvid/zdvidinfo.h"
#include "zobject3dscan.h"

TEST(ZDvidInfo, Basic)
{
  {
    ZDvidInfo info;
    ASSERT_FALSE(info.isValid());
    info.setFromJsonString(
          "{\"Base\":{\"TypeName\":\"uint8blk\","
          "\"TypeURL\":\"github.com/janelia-flyem/dvid/datatype/imageblk/uint8.go\","
          "\"TypeVersion\":\"0.2\",\"DataUUID\":\"8b4c3b5c6c05490394d0d3a396f1ec05\","
          "\"Name\":\"grayscale\",\"RepoUUID\":\"42806a3ff2ff4a0c9831c4461d059773\","
          "\"Compression\":\"LZ4 compression, level -1\","
          "\"Checksum\":\"No checksum\",\"Syncs\":[],\"Versioned\":true,"
          "\"KVStore\":\"basholeveldb @ /Users/zhaot/Work/neutu/neurolabi/data/_dvid/dbs/test\","
          "\"LogStore\":\"write logs @ /Users/zhaot/Work/neutu/neurolabi/data/_dvid/mutlog\","
          "\"Tags\":{}},\"Extended\":{\"Values\":[{\"DataType\":\"uint8\","
          "\"Label\":\"uint8\"}],\"Interpolable\":true,"
          "\"BlockSize\":[32,32,32],\"VoxelSize\":[8,8,8],"
          "\"VoxelUnits\":[\"nanometers\",\"nanometers\",\"nanometers\"],"
          "\"MinPoint\":[1,2,3],\"MaxPoint\":[2047,2047,2047],"
          "\"MinIndex\":[0,0,0],\"MaxIndex\":[63,63,63],\"Background\":0},"
          "\"Extents\":{\"MinPoint\":[1,2,3],\"MaxPoint\":[2047,2047,2047]}}");

    ASSERT_TRUE(info.isValid());
    ASSERT_TRUE(info.isLz4Compression());
    ASSERT_FALSE(info.isJpegCompression());
    ASSERT_EQ(1, info.getMinX());
    ASSERT_EQ(2047, info.getMaxX());
    ASSERT_EQ(2, info.getMinY());
    ASSERT_EQ(2047, info.getMaxY());
    ASSERT_EQ(3, info.getMinZ());
    ASSERT_EQ(2047, info.getMaxZ());
    ASSERT_EQ(ZIntPoint(32, 32, 32), info.getBlockSize());
    info.setCompression("jpeg compression");
    ASSERT_FALSE(info.isLz4Compression());
    ASSERT_TRUE(info.isJpegCompression());
  }
}

TEST(ZDvidInfo, BlockIndex)
{
  {
    ZDvidInfo info;
//    info.print();
//    std::cout << std::endl;
    info.setFromJsonString("{ "
                           " \"MinPoint\": [1, 2, 3], "
                           " \"BlockSize\": [16, 32, 64],"
                           " \"MaxPoint\": [1000, 2000, 3000],"
                           " \"MinIndex\": [4, 5, 6],"
                           " \"MaxIndex\": [1000, 2000, 3000]"
                           "}");
//    info.print();

    ASSERT_EQ(1, info.getStartCoordinates().getX());
    ASSERT_EQ(2, info.getStartCoordinates().getY());
    ASSERT_EQ(3, info.getStartCoordinates().getZ());

    ASSERT_EQ(1000, info.getEndCoordinates().getX());
    ASSERT_EQ(2000, info.getEndCoordinates().getY());
    ASSERT_EQ(3000, info.getEndCoordinates().getZ());

    ASSERT_EQ(16, info.getBlockSize().getX());
    ASSERT_EQ(32, info.getBlockSize().getY());
    ASSERT_EQ(64, info.getBlockSize().getZ());

    ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockIndex(0, 0, 0));
    ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-1, -1, -1));
    ASSERT_EQ(ZIntPoint(1, 1, 1), info.getBlockIndex(16, 32, 64));
    ASSERT_EQ(ZIntPoint(-1, -1, -1), info.getBlockIndex(-16, -32, -64));
    ASSERT_EQ(ZIntPoint(-2, -2, -2), info.getBlockIndex(-17, -33, -65));
    ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockCoord(0, 0, 0));
    ASSERT_EQ(ZIntPoint(16, 32, 64), info.getBlockCoord(1, 1, 1));
    ZIntCuboid box = info.getBlockBox(1, 2, 3);
    ASSERT_EQ(ZIntPoint(16, 64, 192), box.getMinCorner());
    ASSERT_EQ(16, box.getWidth());
    ASSERT_EQ(32, box.getHeight());
    ASSERT_EQ(64, box.getDepth());

    ASSERT_TRUE(info.isValidBlockIndex(ZIntPoint(100, 20, 30)));
    ASSERT_FALSE(info.isValidBlockIndex(ZIntPoint(100, 20, 3)));

    ZObject3dScan obj;
    obj.addSegment(-1, -2, -1, 10);
    ZObject3dScan obj2 = info.getBlockIndex(obj);
    ASSERT_EQ(2, int(obj2.getVoxelNumber()));
    ASSERT_TRUE(obj2.contains(-1, -1, -1));
    ASSERT_TRUE(obj2.contains(0, -1, -1));
  }

  {
    ZDvidInfo info;
    info.setFromJsonString(
          "{\"Base\":{\"TypeName\":\"uint8blk\","
          "\"TypeURL\":\"github.com/janelia-flyem/dvid/datatype/imageblk/uint8.go\","
          "\"TypeVersion\":\"0.2\",\"DataUUID\":\"8b4c3b5c6c05490394d0d3a396f1ec05\","
          "\"Name\":\"grayscale\",\"RepoUUID\":\"42806a3ff2ff4a0c9831c4461d059773\","
          "\"Compression\":\"LZ4 compression, level -1\","
          "\"Checksum\":\"No checksum\",\"Syncs\":[],\"Versioned\":true,"
          "\"KVStore\":\"basholeveldb @ /Users/zhaot/Work/neutu/neurolabi/data/_dvid/dbs/test\","
          "\"LogStore\":\"write logs @ /Users/zhaot/Work/neutu/neurolabi/data/_dvid/mutlog\","
          "\"Tags\":{}},\"Extended\":{\"Values\":[{\"DataType\":\"uint8\","
          "\"Label\":\"uint8\"}],\"Interpolable\":true,"
          "\"BlockSize\":[32,32,32],\"VoxelSize\":[8,8,8],"
          "\"VoxelUnits\":[\"nanometers\",\"nanometers\",\"nanometers\"],"
          "\"MinPoint\":[0,0,0],\"MaxPoint\":[2047,2047,2047],"
          "\"MinIndex\":[0,0,0],\"MaxIndex\":[63,63,63],\"Background\":0},"
          "\"Extents\":{\"MinPoint\":[0,0,0],\"MaxPoint\":[2047,2047,2047]}}");

    ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockIndex(0, 0, 0));
    ASSERT_EQ(ZIntPoint(0, 0, 0), info.getBlockIndex(10, 20, 30));
    ASSERT_EQ(ZIntPoint(0, 0, 1), info.getBlockIndex(0, 0, 32));
    ASSERT_EQ(ZIntPoint(1, 2, 3), info.getBlockIndex(32, 64, 96));
    ASSERT_EQ(5, info.getBlockLevel());

    ZObject3dScan obj = info.getBlockIndex(ZIntCuboid(32, 64, 96, 64, 128, 192));
    ASSERT_EQ(24, int(obj.getVoxelNumber()));
//    obj.print();
  }

}

#endif


#endif // ZDVIDINFOTEST_H
