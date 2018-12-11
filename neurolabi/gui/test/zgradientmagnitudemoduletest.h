#ifndef ZTESTTEST_H
#define ZTESTTEST_H
#include<vector>
#include "ztestheader.h"
#include "sandbox/zgradientmagnitudemodule.h"
#include "zstack.hxx"

#ifdef _USE_GTEST_


TEST(GRADIENTMAGNITUDEMODULE,EMPTY)
{
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  EXPECT_NO_THROW(context.run(0,0));
}


TEST(GRADIENTMAGNITUDEMODULE,ONE)
{
  ZStack* in=new ZStack(GREY,1,1,1,1);
  ZStack* out=new ZStack(GREY,1,1,1,1);
  uint8_t *po=out->array8();
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out);
  EXPECT_EQ(0,po[0]);
  delete in;
  delete out;
}


TEST(GRADIENTMAGNITUDEMODULE,TWO)
{
  ZStack* in=new ZStack(GREY,2,1,1,1);
  ZStack* out=new ZStack(GREY,2,1,1,1);
  uint8_t *pi=in->array8(),*po=out->array8();
  pi[0]=0;
  pi[1]=255;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out);
  EXPECT_EQ(255,po[0]);
  EXPECT_EQ(255,po[1]);
  delete in;
  delete out;

  in=new ZStack(GREY,1,2,1,1);
  out=new ZStack(GREY,1,2,1,1);
  pi=in->array8(),po=out->array8();
  pi[0]=0;
  pi[1]=255;
  context.run(in,out);
  EXPECT_EQ(255,po[0]);
  EXPECT_EQ(255,po[1]);
  delete in;
  delete out;

  in=new ZStack(GREY,1,1,2,1);
  out=new ZStack(GREY,1,1,2,1);
  pi=in->array8(),po=out->array8();
  pi[0]=0;
  pi[1]=255;
  context.run(in,out);
  EXPECT_EQ(255,po[0]);
  EXPECT_EQ(255,po[1]);
  delete in;
  delete out;
}

TEST(GRADIENTMAGNITUDEMODULE,THREE)
{
  ZStack* in=new ZStack(GREY,3,2,1,1);
  ZStack* out=new ZStack(GREY,3,2,1,1);
  uint8_t *pi=in->array8(),*po=out->array8();
  pi[0]=0;
  pi[1]=3;
  pi[2]=6;
  pi[3]=4;
  pi[4]=7;
  pi[5]=10;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out);
  for(uint i=0;i<out->getVoxelNumber();++i)
  {
    EXPECT_EQ(5,po[i]);
  }
  delete in;
  delete out;
}

TEST(GRADIENTMAGNITUDEMODULE,REVERSE)
{
  ZStack* in=new ZStack(GREY,3,2,1,1);
  ZStack* out=new ZStack(GREY,3,2,1,1);
  uint8_t *pi=in->array8(),*po=out->array8();
  pi[0]=0;
  pi[1]=3;
  pi[2]=6;
  pi[3]=4;
  pi[4]=7;
  pi[5]=10;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out,true);
  for(uint i=0;i<out->getVoxelNumber();++i)
  {
    EXPECT_EQ(250,po[i]);
  }
}


TEST(GRADIENTMAGNITUDEMODULE,FLOAT)
{
  ZStack* in=new ZStack(FLOAT32,2,1,1,1);
  ZStack* out=new ZStack(FLOAT32,2,1,1,1);
  float *pi=in->array32(),*po=out->array32();
  pi[0]=0;
  pi[1]=1000;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out,true);
  EXPECT_FLOAT_EQ(FLT_MAX-1000,po[0]);
  EXPECT_FLOAT_EQ(FLT_MAX-1000,po[1]);
  delete in;
  delete out;
}


/*
TEST(GRADIENTMAGNITUDEMODULE,RGB)
{
  ZStack* in=new ZStack(COLOR,1,1,2,1);
  ZStack* out=new ZStack(COLOR,1,1,2,1);
  color_t *pi=in->arrayc(),*po=out->arrayc();
  pi[0][0]=pi[0][1]=pi[0][2]=0;
  pi[1][0]=123;
  pi[1][1]=90;
  pi[1][2]=110;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out);
  EXPECT_EQ(123,po[0][0]);
  EXPECT_EQ(90,po[0][1]);
  EXPECT_EQ(110,po[0][2]);
  EXPECT_EQ(123,po[1][0]);
  EXPECT_EQ(90,po[1][1]);
  EXPECT_EQ(110,po[1][2]);
  delete in;
  delete out;

  in=new ZStack(COLOR,1,1,2,1);
  out=new ZStack(COLOR,1,1,2,1);
  pi=in->arrayc(),po=out->arrayc();
  pi[0][0]=pi[0][1]=pi[0][2]=0;
  pi[1][0]=123;
  pi[1][1]=90;
  pi[1][2]=110;
  context.run(in,out,true);
  EXPECT_EQ(132,po[0][0]);
  EXPECT_EQ(165,po[0][1]);
  EXPECT_EQ(145,po[0][2]);
  EXPECT_EQ(132,po[1][0]);
  EXPECT_EQ(165,po[1][1]);
  EXPECT_EQ(145,po[1][2]);
  delete in;
  delete out;
}
*/

TEST(GRADIENTMAGNITUDEMODULE,NORMAL)
{
  ZStack* in=new ZStack(GREY,3,2,1,1);
  ZStack* out=new ZStack(GREY,3,2,1,1);

  uint8_t *pi=in->array8(),*po=out->array8();
  pi[0]=0;
  pi[1]=3;
  pi[2]=6;
  pi[3]=4;
  pi[4]=7;
  pi[5]=10;
  GradientStrategyContext context(GradientStrategyContext::SIMPLE);
  context.run(in,out,true);
  for(uint i=0;i<out->getVoxelNumber();++i)
  {
    EXPECT_EQ(250,po[i]);
  }
//  context.run(in,out,false,true,0.5);
//  for(uint i=0;i<out->getVoxelNumber();++i)
//  {
//    EXPECT_EQ(int(5*0.5+0.5*pi[i]),po[i]);
//  }
//  context.run(in,out,true,true,0.4);
//  for(uint i=0;i<out->getVoxelNumber();++i)
//  {
//    EXPECT_EQ(int(250*0.6+0.4*pi[i]),po[i]);
//  }
}

#endif
#endif // ZTESTTEST_H
