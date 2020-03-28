#ifndef ES_HTTP_REQUEST_H
#include <ESHttpRequest.h>
#endif

#ifndef ES_HTTP_RESPONSE_H
#include <ESHttpResponse.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#include <gtest/gtest.h>

using namespace ES;

TEST(HttpCommon, Alignment) {
  ESB::UInt32 cacheLineSize = ESB::SystemConfig::Instance().cacheLineSize();
  EXPECT_EQ(cacheLineSize*2, sizeof(HttpRequest));
  EXPECT_EQ(cacheLineSize, sizeof(HttpResponse));
  EXPECT_EQ(cacheLineSize, sizeof(HttpRequestUri));
  EXPECT_EQ(cacheLineSize, sizeof(HttpHeader));
}
