#include <iostream>
#include <core/app-template.hh>
#include <core/sharded.hh>
#include <smf/rpc_server.h>
#include "verve.smf.fb.h"

namespace po = boost::program_options;

class MemoryNodeX {
 public:
  seastar::future<> begin() {
    return seastar::make_ready_future<>();
  }

  seastar::future<> stop() {
    return seastar::make_ready_future<>();
  }
};

class MemoryNodeService : public verve::rpc::MemoryNode {
 public:
  explicit MemoryNodeService(seastar::sharded<MemoryNodeX> *mem_node) :
    mem_node_(mem_node)
  {}

  seastar::future<smf::rpc_typed_envelope<verve::rpc::Response>> Prepare(
      smf::rpc_recv_typed_context<verve::rpc::Request>&& req) final {

    smf::rpc_typed_envelope<verve::rpc::Response> data;

    auto reads = req->reads();
    for (auto it = reads->begin(); it != reads->end(); it++) {
      std::cout << "read: " << it->offset() << "/" << it->length() << std::endl;
    }

    // return the same payload
    if (req) { data.data->name = req->name()->c_str(); }

    data.envelope.set_status(200);
    return seastar::make_ready_future<
      smf::rpc_typed_envelope<verve::rpc::Response>>(std::move(data));
  }

 private:
  seastar::sharded<MemoryNodeX> *mem_node_;
};

int main(int args, char **argv)
{
  seastar::sharded<smf::rpc_server> rpc;
  seastar::sharded<MemoryNodeX> mem_node;

  seastar::app_template app;
  app.add_options()
    ("ip", po::value<std::string>()->default_value("127.0.0.1"), "ip to connect to")
    ("port", po::value<uint16_t>()->default_value(20776), "port for service")
  ;

  return app.run_deprecated(args, argv, [&] {
    seastar::engine().at_exit([&mem_node] { return mem_node.stop(); });
    seastar::engine().at_exit([&rpc] { return rpc.stop(); });

    auto& config = app.configuration();

    return mem_node.start().then([&mem_node] {
      return mem_node.invoke_on_all(&MemoryNodeX::begin);
    }).then([&rpc, &config] {
      smf::rpc_server_args args;
      args.ip = config["ip"].as<std::string>().c_str();
      args.rpc_port = config["port"].as<uint16_t>();
      return rpc.start(args);
    }).then([&rpc, &mem_node] {
      return rpc.invoke_on_all([&](smf::rpc_server& server) {
        server.register_service<MemoryNodeService>(&mem_node);
      });
    }).then([&rpc] {
      return rpc.invoke_on_all(&smf::rpc_server::start);
    });
  });
}
