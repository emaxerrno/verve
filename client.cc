#include <iostream>
#include <core/app-template.hh>
#include <core/sharded.hh>
#include "verve.smf.fb.h"

namespace po = boost::program_options;

class connection {
 public:
  connection(seastar::ipv4_addr server)
  {
    smf::rpc_client_opts opts{};
    opts.server_addr = server;
    client = verve::rpc::MemoryNodeClient::make_shared(std::move(opts));
  }

  seastar::future<> connect() {
    return client->connect();
  }

  seastar::future<> stop() {
    return client->stop();
  }

  seastar::shared_ptr<verve::rpc::MemoryNodeClient> client;
};

class client {
 public:
  unsigned reqs;
  unsigned cons;
  std::vector<std::unique_ptr<connection>> conns;

  client(unsigned reqs, unsigned cons, seastar::ipv4_addr server) :
    reqs(reqs),
    cons(cons)
  {
    for (auto i = 0u; i < cons; i++) {
      conns.push_back(std::make_unique<connection>(server));
    }
  }

  seastar::future<> connect() {
    return seastar::do_for_each(conns.begin(), conns.end(),
      [](auto& c) { return c->connect(); });
  }

  seastar::future<> run() {
    return seastar::do_for_each(
      boost::counting_iterator<int>(0),
      boost::counting_iterator<int>(5),
      [&](int i) mutable {
        smf::rpc_typed_envelope<verve::rpc::Request> req;
        req.data->name = "foo";
        std::vector<verve::rpc::Extent> extents;
        extents.push_back({10, 20});
        extents.push_back({20, 30});
        req.data->reads = extents;
        auto e = req.serialize_data();
        return conns[0]->client->Prepare(std::move(e)).then([](auto ret) {
          return seastar::make_ready_future<>();
        });
    });
  }

  seastar::future<> stop() {
    return seastar::do_for_each(conns.begin(), conns.end(),
      [](auto& c) { return c->stop(); });
  }
};

int main(int args, char **argv)
{
  seastar::sharded<client> sharded_client;

  seastar::app_template app;
  app.add_options()
    ("ip", po::value<std::string>()->default_value("127.0.0.1"), "ip to connect to")
    ("port", po::value<uint16_t>()->default_value(20776), "port for service")
    ("req-num", po::value<uint32_t>()->default_value(1000), "reqs per connection")
    ("concurrency", po::value<uint32_t>()->default_value(10), "num green threads per core")
  ;

  return app.run(args, argv, [&] {
    seastar::engine().at_exit([&] { return sharded_client.stop(); });

    auto& config = app.configuration();
    auto reqs = config["req-num"].as<uint32_t>();
    auto cons = config["concurrency"].as<uint32_t>();
    auto ip = config["ip"].as<std::string>().c_str();
    auto port = config["port"].as<uint16_t>();

    auto addr = seastar::ipv4_addr{ip, port};

    return sharded_client.start(reqs, cons, addr).then([&] {
      return sharded_client.invoke_on_all(&client::connect);
    }).then([&] {
      return sharded_client.invoke_on_all(&client::run);
    }).then([&] {
      return sharded_client.stop();
    }).then([&] {
      return seastar::make_ready_future<int>(0);
    });
  });
}
