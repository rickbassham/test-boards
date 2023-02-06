#include <libindi/json.h>
#include <curl/curl.h>

struct response_t
{
    char *response;
    size_t size;
};

static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize        = size * nmemb;
    struct response_t *mem = (struct response_t *)userp;

    char *ptr = (char *)realloc(mem->response, mem->size + realsize + 1);
    if (ptr == NULL)
        return 0; /* out of memory! */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

nlohmann::json get_json(const char *url)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        struct response_t chunk = { .response = nullptr, .size = 0 };

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLcode::CURLE_OK)
        {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            if (http_code == 200)
                return nlohmann::json::parse(chunk.response);
        }
    }

    return nlohmann::json(nullptr);
}

nlohmann::json put_json(const char* url, const nlohmann::json& body)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        struct response_t chunk = { .response = nullptr, .size = 0 };

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.dump().c_str());

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLcode::CURLE_OK)
        {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            if (http_code == 200)
                return nlohmann::json::parse(chunk.response);
        }
    }

    return nlohmann::json(nullptr);
}

nlohmann::json put_json(const char* url, const std::map<std::string, std::string> &body)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl)
    {
        struct response_t chunk = { .response = nullptr, .size = 0 };

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

        if (body.size() > 0)
        {
            auto form = curl_mime_init(curl);
            for (auto& [key, value] : body)
            {
                auto field = curl_mime_addpart(form);
                curl_mime_name(field, key.c_str());
                curl_mime_data(field, value.c_str(), CURL_ZERO_TERMINATED);
            }

            curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
        }

        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLcode::CURLE_OK)
        {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

            if (http_code == 200)
                return nlohmann::json::parse(chunk.response);
        }
    }

    return nlohmann::json(nullptr);
}