#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <envoy/service/listener/v3/lds.grpc.pb.h>

class ListenerDiscoveryService : public envoy::service::listener::v3::ListenerDiscoveryService::Service {
  virtual grpc::Status StreamListeners(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<envoy::service::discovery::v3::DiscoveryResponse,
                               envoy::service::discovery::v3::DiscoveryRequest>* stream) override {
    envoy::service::discovery::v3::DiscoveryRequest request;
    while (stream->Read(&request)) {
      std::cerr << "Read request" << std::endl;
      if (0 != request.type_url().compare("hi")) {
        std::cerr << "got unexpected type url" << std::endl;
      }
      envoy::service::discovery::v3::DiscoveryResponse response;
      stream->Write(response);
      std::cerr << "Sent response" << std::endl;
    }
    return grpc::Status::OK;
  }
};

int main(int argc, const char** argv) {
  std::string server_address("0.0.0.0:50051");
  ListenerDiscoveryService service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();

  return 0;
}
