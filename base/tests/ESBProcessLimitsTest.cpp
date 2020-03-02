#ifndef ESB_PROCESS_LIMITS_H
#include <ESBProcessLimits.h>
#endif

#include <gtest/gtest.h>

using namespace ESB;

TEST(ProcessLimits, Sockets) {
  UInt32 softLimit = ProcessLimits::GetSocketSoftMax();
  UInt32 hardLimit = ProcessLimits::GetSocketHardMax();
  Error error = ProcessLimits::SetSocketSoftMax(softLimit);

  EXPECT_EQ(error, ESB_SUCCESS);
  EXPECT_EQ(softLimit, ProcessLimits::GetSocketSoftMax());
  EXPECT_GE(hardLimit, softLimit);

  error = ProcessLimits::SetSocketSoftMax(hardLimit);
  EXPECT_EQ(error, ESB_SUCCESS);
  EXPECT_EQ(hardLimit, ProcessLimits::GetSocketSoftMax());
}
