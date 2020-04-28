#ifndef ZMESHTEST_H
#define ZMESHTEST_H

#include "ztestheader.h"
#include "zmesh.h"

#ifdef _USE_GTEST_

TEST(ZMesh, Properties)
{
  {
    ZMesh mesh;
    ASSERT_TRUE(mesh.boundBox().empty());
  }

  {
    ZMesh mesh = ZMesh::CreateCube();
    ASSERT_FALSE(mesh.boundBox().empty());
    ASSERT_DOUBLE_EQ(0.0, mesh.boundBox().minCorner()[0]);
    ASSERT_DOUBLE_EQ(1.0, mesh.boundBox().maxCorner()[0]);

    mesh.translate(1.0, 2.0, 3.0);
    ASSERT_DOUBLE_EQ(2.0, mesh.boundBox().minCorner()[1]);
    ASSERT_DOUBLE_EQ(3.0, mesh.boundBox().maxCorner()[1]);

    mesh.swapXZ();
    ASSERT_DOUBLE_EQ(3.0, mesh.boundBox().minCorner()[0]);
    ASSERT_DOUBLE_EQ(4.0, mesh.boundBox().maxCorner()[0]);

    ZMesh mesh2 = ZMesh::CreateCube();
    mesh2.translate(10, 20, 30);
    ASSERT_DOUBLE_EQ(10.0, mesh2.boundBox().minCorner()[0]);
    ASSERT_DOUBLE_EQ(11.0, mesh2.boundBox().maxCorner()[0]);

    mesh.append(mesh2);
    ASSERT_DOUBLE_EQ(3.0, mesh.boundBox().minCorner()[0]);
    ASSERT_DOUBLE_EQ(11.0, mesh.boundBox().maxCorner()[0]);
  }
}

TEST(ZMesh, Assign)
{
  ZMesh mesh;
  mesh.setSource("test");

  ZMesh *mesh2 = mesh.clone();
  ASSERT_EQ("test", mesh2->getSource());

  ZMesh mesh3;
  mesh3 = mesh;
  ASSERT_EQ("test", mesh3.getSource());

  ZMesh mesh4(std::move(mesh));
  ASSERT_EQ("test", mesh4.getSource());
}


#endif

#endif // ZMESHTEST_H
