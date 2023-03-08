#include <pistache/common.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/http_headers.h>
#include <pistache/net.h>
#include <pistache/peer.h>
#include "handlers/filter.hpp"

using namespace Pistache;

struct LoadMonitor
{
    LoadMonitor(const std::shared_ptr<Http::Endpoint>& endpoint)
        : endpoint_(endpoint)
        , interval(std::chrono::seconds(1))
    { }

    void setInterval(std::chrono::seconds secs)
    {
        interval = secs;
    }

    void start()
    {
        shutdown_ = false;
        thread    = std::make_unique<std::thread>([this] { run(); });
    }

    void shutdown()
    {
        shutdown_ = true;
    }

    ~LoadMonitor()
    {
        shutdown_ = true;
        if (thread)
            thread->join();
    }

private:
    std::shared_ptr<Http::Endpoint> endpoint_;
    std::unique_ptr<std::thread> thread;
    std::chrono::seconds interval;

    std::atomic<bool> shutdown_;

    void run()
    {
        Tcp::Listener::Load old;
        while (!shutdown_)
        {
            if (!endpoint_->isBound())
                continue;

            endpoint_->requestLoad(old).then([&](const Tcp::Listener::Load& load) {
                old = load;

                double global = load.global;
                if (global > 100)
                    global = 100;

                if (global > 1)
                    std::cout << "Global load is " << global << "%" << std::endl;
                else
                    std::cout << "Global load is 0%" << std::endl;
            },
                                             Async::NoExcept);

            std::this_thread::sleep_for(std::chrono::seconds(interval));
        }
    }
};

int main(int argc, char* argv[])
{
    Port port(9080);

    int thr = 2;

    if (argc >= 2)
    {
        port = static_cast<uint16_t>(std::stol(argv[1]));

        if (argc == 3)
            thr = std::stoi(argv[2]);
    }

    Address addr(Ipv4::any(), port);

    std::cout << "Cores = " << hardware_concurrency() << std::endl;
    std::cout << "Using " << thr << " threads" << std::endl;

    auto server = std::make_shared<Http::Endpoint>(addr);

    auto opts = Http::Endpoint::options()
                    .threads(thr);
    server->init(opts);
    server->setHandler(Http::make_handler<FilterHandler>());
    server->serve();
}