#ifndef ZDOCPLAYERTEST_H
#define ZDOCPLAYERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zdocplayer.h"
#include "zobject3d.h"

#ifdef _USE_GTEST_

TEST(ZDocPlayer, Property)
{
  ZDocPlayer player;

  ASSERT_EQ(NULL, player.getData());
  ASSERT_FALSE(player.hasRole(ZStackObjectRole::ROLE_NONE));
  ASSERT_FALSE(player.hasRole(ZStackObjectRole::ROLE_DISPLAY));
  ASSERT_FALSE(player.hasRole(ZStackObjectRole::ROLE_SEED));
  ASSERT_FALSE(player.hasRole(ZStackObjectRole::ROLE_TMP_RESULT));
}

TEST(ZDocPlayerList, General)
{
  ZDocPlayerList playerList;
  playerList.append(new ZDocPlayer(new ZObject3d));
  ASSERT_EQ(1, playerList.size());
  ASSERT_TRUE(playerList.hasPlayer(ZStackObjectRole::ROLE_NONE));
  ASSERT_FALSE(playerList.hasPlayer(ZStackObjectRole::ROLE_3DPAINT));

  playerList.removePlayer(ZStackObjectRole::ROLE_NONE);
  ASSERT_EQ(0, playerList.size());

  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_NONE);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_DISPLAY);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_NONE);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_NONE);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_SEED);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_DISPLAY);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_DISPLAY | ZStackObjectRole::ROLE_SEED);
    playerList.append(new ZDocPlayer(obj));
  }
  {
    ZObject3d *obj = new ZObject3d;
    obj->setRole(ZStackObjectRole::ROLE_DISPLAY |
                 ZStackObjectRole::ROLE_MANAGED_OBJECT);
    playerList.append(new ZDocPlayer(obj));
  }


  ASSERT_TRUE(playerList.hasPlayer(
                ZStackObjectRole::ROLE_DISPLAY | ZStackObjectRole::ROLE_SEED));
  ASSERT_TRUE(playerList.hasPlayer(
                ZStackObjectRole::ROLE_MANAGED_OBJECT));
  ASSERT_TRUE(playerList.hasPlayer(
               ZStackObjectRole::ROLE_DISPLAY | ZStackObjectRole::ROLE_MANAGED_OBJECT));
  ASSERT_FALSE(playerList.hasPlayer(
               ZStackObjectRole::ROLE_SEED | ZStackObjectRole::ROLE_MANAGED_OBJECT));
  ASSERT_EQ(3, playerList.getPlayerList(ZStackObjectRole::ROLE_NONE).size());
  ASSERT_EQ(4, playerList.getPlayerList(ZStackObjectRole::ROLE_DISPLAY).size());
  ASSERT_EQ(2, playerList.getPlayerList(ZStackObjectRole::ROLE_SEED).size());
  ASSERT_EQ(1, playerList.getPlayerList(
              ZStackObjectRole::ROLE_SEED | ZStackObjectRole::ROLE_DISPLAY).size());

  ZStackObjectRole::TRole role =
      playerList.removePlayer(ZStackObjectRole::ROLE_DISPLAY);
  ASSERT_EQ(ZStackObjectRole::ROLE_DISPLAY | ZStackObjectRole::ROLE_SEED |
            ZStackObjectRole::ROLE_MANAGED_OBJECT,
            role);
  ASSERT_EQ(4, playerList.size());
}

#endif

#endif // ZDOCPLAYERTEST_H
