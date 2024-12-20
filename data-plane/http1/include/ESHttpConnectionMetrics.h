#ifndef ES_HTTP_CONNECTION_METRICS_H
#define ES_HTTP_CONNECTION_METRICS_H

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

#ifndef ESB_SHARED_AVERAGING_COUNTER_H
#include <ESBSharedAveragingCounter.h>
#endif

namespace ES {

class HttpConnectionMetrics {
 public:
  HttpConnectionMetrics();
  ~HttpConnectionMetrics();

  ESB::SharedInt &totalConnections() { return _totalConnections; }
  const ESB::SharedInt &totalConnections() const { return _totalConnections; }
  ESB::SharedAveragingCounter &averageTransactionsPerConnection() { return _averageTransactionsPerConnection; }
  const ESB::SharedAveragingCounter &averageTransactionsPerConnection() const {
    return _averageTransactionsPerConnection;
  }

 private:
  ESB::SharedInt _totalConnections;
  ESB::SharedAveragingCounter _averageTransactionsPerConnection;

  ESB_DISABLE_AUTO_COPY(HttpConnectionMetrics);
};

}  // namespace ES

#endif