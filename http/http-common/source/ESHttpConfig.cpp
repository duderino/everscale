#ifndef ES_HTTP_CONFIG_H
#include <ESHttpConfig.h>
#endif

#ifndef ESB_SYSTEM_CONFIG_H
#include <ESBSystemConfig.h>
#endif

#ifndef ESB_DISCARD_ALLOCATOR_H
#include <ESBDiscardAllocator.h>
#endif

#ifndef ESB_BUFFER_POOL_H
#include <ESBBufferPool.h>
#endif

#include <cmath>

namespace ES {

HttpConfig HttpConfig::_Instance;

HttpConfig::HttpConfig() : _connectionPoolBuckets(7919U) {
  const ESB::UInt32 bufsz = ESB_PAGE_SIZE * 8U;
  const ESB::UInt32 bufs = 1000U;
  const ESB::UInt32 chunksz = ESB::DiscardAllocator::SizeofChunk(ESB_CACHE_LINE_SIZE);
  const ESB::UInt32 bufOverhead = ESB_ALIGN(sizeof(ESB::Buffer), ESB_CACHE_LINE_SIZE);
  const ESB::UInt32 chunkOverhead = (ESB::UInt32)ceil((double)chunksz / bufs);

  _ioBufferSize = bufsz - bufOverhead - chunkOverhead;
  _ioBufferChunkSize = bufs * bufsz - chunksz;
}

HttpConfig::~HttpConfig() {}

}  // namespace ES
