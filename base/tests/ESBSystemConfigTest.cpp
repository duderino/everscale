#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(SystemConfig, Sockets) {
  UInt32 softLimit = SystemConfig::Instance().socketSoftMax();
  UInt32 hardLimit = SystemConfig::Instance().socketHardMax();
  Error error = SystemConfig::Instance().setSocketSoftMax(softLimit);

  EXPECT_EQ(error, ESB_SUCCESS);
  EXPECT_EQ(softLimit, SystemConfig::Instance().socketSoftMax());
  EXPECT_GE(hardLimit, softLimit);

  error = SystemConfig::Instance().setSocketSoftMax(hardLimit);
  EXPECT_EQ(error, ESB_SUCCESS);
  EXPECT_EQ(hardLimit, SystemConfig::Instance().socketSoftMax());
}
