/**
 * @file filter.hpp
 * @brief FilterHandler class definition
 * @author Doğukan Öksüz
 */

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <curl/curl.h>
#include <algorithm>

using namespace Pistache;

class FilterHandler
{
public:
    explicit FilterHandler(Address addr)
        : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    {
    }

    void init(size_t thr = 2)
    {
        auto opts = Http::Endpoint::options()
                        .threads(static_cast<int>(thr));
        httpEndpoint->init(opts);
        setupRoutes();
    }

    void start()
    {
        httpEndpoint->setHandler(router.handler());
        httpEndpoint->serve();
    }

private:
    void setupRoutes()
    {
        using namespace Rest;

        Routes::Get(router, "/filter", Routes::bind(&FilterHandler::filter, this));
    }

    void unescape_url(std::string &url)
    {
        CURL *curl = curl_easy_init();
        if (curl)
        {
            char *output = curl_easy_unescape(curl, url.c_str(), url.length(), NULL);
            url = output;
            curl_free(output);
            curl_easy_cleanup(curl);
        }
    }

    void filter(const Rest::Request &request, Http::ResponseWriter response)
    {
        auto query = request.query();
        auto filter = query.get("filter");

        if (filter)
        {
            auto filter_string = *std::move(filter);
            unescape_url(filter_string);
            const char *unparsed_json = filter_string.c_str();

            rapidjson::Document document;
            document.Parse(unparsed_json);

            if (document.HasParseError())
            {
                response.send(Http::Code::Bad_Request, "Invalid JSON");
                return;
            }

            std::string parsed_value = "";
            if (document.IsArray())
            {
                for (auto &v : document.GetArray())
                {
                    if (v.IsObject())
                    {
                        std::string filter_value = "";

                        for (auto &m : v.GetObject())
                        {
                            if (strcmp(m.name.GetString(), "key") == 0)
                            {
                                filter_value += m.value.GetString();
                                filter_value += ": ";
                                continue;
                            }

                            if (strcmp(m.name.GetString(), "value") == 0)
                            {
                                if (m.value.IsString())
                                {
                                    filter_value += m.value.GetString();
                                    filter_value += "\n";
                                }

                                if (m.value.IsArray())
                                {
                                    filter_value += "[\n";
                                    for (auto &n : m.value.GetArray())
                                    {
                                        if (n.IsString())
                                        {
                                            filter_value += "\t";
                                            filter_value += n.GetString();
                                            filter_value += ",";
                                            filter_value += "\n";
                                        }
                                    }
                                    filter_value += "]\n";
                                }
                            }
                        }

                        parsed_value += filter_value;
                        std::cout << filter_value << std::endl;
                    }
                }
            }

            response.send(Http::Code::Ok, parsed_value);
            return;
        }
        response.send(Http::Code::Bad_Request, "No filter provided");
    }

    std::shared_ptr<Http::Endpoint> httpEndpoint;
    Rest::Router router;
};
