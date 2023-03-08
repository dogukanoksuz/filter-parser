#include <pistache/common.h>
#include <pistache/cookie.h>
#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/http_headers.h>
#include <pistache/net.h>
#include <pistache/peer.h>

using namespace Pistache;

class FilterHandler : public Http::Handler
{
    HTTP_PROTOTYPE(FilterHandler)

    void onRequest(
        const Http::Request& req,
        Http::ResponseWriter response) override
    {
        if (req.resource() == "/filter") {
            if (req.method() == Http::Method::Get) {
                using namespace Http;

                //const auto& query = req.query();

                response.send(Http::Code::Ok, "PONG");
            }
        }
        else if (req.resource() == "/echo")
        {
            if (req.method() == Http::Method::Post)
            {
                response.send(Http::Code::Ok, req.body(), MIME(Text, Plain));
            }
            else
            {
                response.send(Http::Code::Method_Not_Allowed);
            }
        }
        else
        {
            response.send(Http::Code::Not_Found);
        }
    }

    void onTimeout(
        const Http::Request& /*req*/,
        Http::ResponseWriter response) override
    {
        response
            .send(Http::Code::Request_Timeout, "Timeout")
            .then([=](ssize_t) {}, PrintException());
    }
};
