#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

namespace ES {

HttpConfig HttpConfig::_Instance;

HttpConfig::HttpConfig()
    : _ioBufferSize(ESB_PAGE_SIZE * 8),
      _ioBufferChunkSize(1000U * _ioBufferSize),
      _parseBufferSize(1024U) {}

HttpConfig::~HttpConfig() {}

}  // namespace ES
