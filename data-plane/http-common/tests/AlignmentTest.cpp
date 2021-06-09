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
  //  TODO: Do pre/post perf comparison before committing any cache optimizations
  //  EXPECT_EQ(ESB_CACHE_LINE_SIZE * 2, sizeof(HttpRequest));
  //  EXPECT_EQ(ESB_CACHE_LINE_SIZE, sizeof(HttpResponse));
  //  EXPECT_EQ(ESB_CACHE_LINE_SIZE, sizeof(HttpRequestUri));
  //  EXPECT_EQ(ESB_CACHE_LINE_SIZE, sizeof(HttpHeader));
}
