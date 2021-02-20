#include <iostream>
#include <memory>
#include <string>

#include <envoy/service/listener/v3/lds.grpc.pb.h>
#include <grpcpp/grpcpp.h>

class ListenerDiscoveryClient {
 public:
  ListenerDiscoveryClient(std::shared_ptr<grpc::Channel> channel)
      : stub_(envoy::service::listener::v3::ListenerDiscoveryService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  grpc::Status Send() {
    // Data we are sending to the server.
    envoy::service::discovery::v3::DiscoveryRequest request;
    request.set_type_url("hi");

    // Container for the data we expect from the server.
    envoy::service::discovery::v3::DiscoveryResponse response;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    grpc::ClientContext context;

    // The actual RPC.
    std::unique_ptr<::grpc::ClientReaderWriter<::envoy::service::discovery::v3::DiscoveryRequest,
                                               ::envoy::service::discovery::v3::DiscoveryResponse>>
        stream = stub_->StreamListeners(&context);

    for (int i = 0; i < 3; ++i) {
      if (!stream->Write(request)) {
        std::cerr << "Cannot send request" << std::endl;
        break;
      }
      std::cerr << "Sent request" << std::endl;

      if (!stream->Read(&response)) {
        std::cerr << "Cannot read response" << std::endl;
        break;
      }

      std::cerr << "Read response" << std::endl;
    }

    stream->WritesDone();
    return stream->Finish();
  }

 private:
  std::unique_ptr<envoy::service::listener::v3::ListenerDiscoveryService::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str("localhost:50051");
  ListenerDiscoveryClient client(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  grpc::Status status = client.Send();

  // Act upon its status.
  if (!status.ok()) {
    std::cerr << status.error_code() << ": " << status.error_message() << std::endl;
    return 1;
  }

  return 0;
}
