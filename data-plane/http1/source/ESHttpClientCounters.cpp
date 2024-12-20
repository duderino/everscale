#ifndef ES_HTTP_CLIENT_COUNTERS_H
#include <ESHttpClientCounters.h>
#endif

namespace ES {
HttpClientCounters::HttpClientCounters() {}
HttpClientCounters::~HttpClientCounters() {}

void HttpClientCounters::incrementStatusCounter(int statusCode, const ESB::Date &start, const ESB::Date &stop) {
  if (100 <= statusCode && 200 > statusCode) {
    responseStatus1xx().record(start, stop);
  } else if (200 <= statusCode && 300 > statusCode) {
    responseStatus2xx().record(start, stop);
  } else if (300 <= statusCode && 400 > statusCode) {
    responseStatus3xx().record(start, stop);
  } else if (400 <= statusCode && 500 > statusCode) {
    responseStatus4xx().record(start, stop);
  } else if (500 <= statusCode && 600 > statusCode) {
    responseStatus5xx().record(start, stop);
  } else {
    responseStatusOther().record(start, stop);
  }

  switch (statusCode) {
    case 200:
      responseStatus200().record(start, stop);
      break;
    case 201:
      responseStatus201().record(start, stop);
      break;
    case 202:
      responseStatus202().record(start, stop);
      break;
    case 204:
      responseStatus204().record(start, stop);
      break;
    case 304:
      responseStatus304().record(start, stop);
      break;
    case 400:
      responseStatus400().record(start, stop);
      break;
    case 401:
      responseStatus401().record(start, stop);
      break;
    case 403:
      responseStatus403().record(start, stop);
      break;
    case 404:
      responseStatus404().record(start, stop);
      break;
    case 410:
      responseStatus410().record(start, stop);
      break;
    case 500:
      responseStatus500().record(start, stop);
      break;
    case 502:
      responseStatus502().record(start, stop);
      break;
    case 503:
      responseStatus503().record(start, stop);
      break;
    case 504:
      responseStatus504().record(start, stop);
      break;
    default:
      break;
  }
}
}  // namespace ES
