#ifndef ZDOCPLAYERTEST_H
#define ZDOCPLAYERTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "zdocplayer.h"

#ifdef _USE_GTEST_

TEST(ZDocPlayer, Property)
{
  ZDocPlayer player;

  ASSERT_EQ(NULL, player.getData());
  ASSERT_TRUE(player.hasRole(ZDocPlayer::ROLE_NONE));
  ASSERT_FALSE(player.hasRole(ZDocPlayer::ROLE_DISPLAY));
  ASSERT_FALSE(player.hasRole(ZDocPlayer::ROLE_SEED));
  ASSERT_FALSE(player.hasRole(ZDocPlayer::ROLE_TMP_RESULT));
}

TEST(ZDocPlayerList, General)
{
  ZDocPlayerList playerList;
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_NONE));
  ASSERT_EQ(1, playerList.size());
  ASSERT_TRUE(playerList.hasPlayer(ZDocPlayer::ROLE_NONE));
  ASSERT_FALSE(playerList.hasPlayer(ZDocPlayer::ROLE_3DPAINT));

  playerList.removePlayer(ZDocPlayer::ROLE_NONE);
  ASSERT_EQ(0, playerList.size());

  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_NONE));
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_DISPLAY));
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_NONE));
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_NONE));
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_SEED));
  playerList.append(new ZDocPlayer(NULL, ZDocPlayer::ROLE_DISPLAY));
  playerList.append(
        new ZDocPlayer(NULL, ZDocPlayer::ROLE_DISPLAY | ZDocPlayer::ROLE_SEED));
  playerList.append(
        new ZDocPlayer(NULL, ZDocPlayer::ROLE_DISPLAY |
                       ZDocPlayer::ROLE_MANAGED_OBJECT));

  ASSERT_TRUE(playerList.hasPlayer(
                ZDocPlayer::ROLE_DISPLAY | ZDocPlayer::ROLE_SEED));
  ASSERT_TRUE(playerList.hasPlayer(
                ZDocPlayer::ROLE_MANAGED_OBJECT));
  ASSERT_TRUE(playerList.hasPlayer(
               ZDocPlayer::ROLE_DISPLAY | ZDocPlayer::ROLE_MANAGED_OBJECT));
  ASSERT_FALSE(playerList.hasPlayer(
               ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_MANAGED_OBJECT));
  ASSERT_EQ(3, playerList.getPlayerList(ZDocPlayer::ROLE_NONE).size());
  ASSERT_EQ(4, playerList.getPlayerList(ZDocPlayer::ROLE_DISPLAY).size());
  ASSERT_EQ(2, playerList.getPlayerList(ZDocPlayer::ROLE_SEED).size());
  ASSERT_EQ(1, playerList.getPlayerList(
              ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_DISPLAY).size());

  ZDocPlayer::TRole role = playerList.removePlayer(ZDocPlayer::ROLE_DISPLAY);
  ASSERT_EQ(ZDocPlayer::ROLE_DISPLAY | ZDocPlayer::ROLE_SEED | ZDocPlayer::ROLE_MANAGED_OBJECT,
            role);
  ASSERT_EQ(4, playerList.size());
}

#endif

#endif // ZDOCPLAYERTEST_H
