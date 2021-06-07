//
// Created by kouta on 4/6/21.
//

#include "server_conn.hpp"
#include "json.hpp"
#include <variant>
#include <string>
#include <functional>
#include "request_tools.hpp"

#ifdef __VITA__
#define VITASDK

#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>
#include <psp2/ctrl.h>
#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>
#include <curlpp/Infos.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <curl/curl.h>
#include <curlpp/cURLpp.hpp>

#endif

void netInit() {
#ifdef __VITA__
    sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

    SceNetInitParam netInitParam;
    int size = 4 * 1024 * 1024;
    netInitParam.memory = malloc(size);
    netInitParam.size = size;
    netInitParam.flags = 0;
    sceNetInit(&netInitParam);

    sceNetCtlInit();
#endif
}

void netTerm() {
#ifdef __VITA__
    sceNetCtlTerm();

    sceNetTerm();

    sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
#endif
}

void httpInit() {
#ifdef __VITA__
    sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

    sceHttpInit(4 * 1024 * 1024);
#endif
    cURLpp::initialize();
}

void httpTerm() {
#ifdef __VITA__
    sceHttpTerm();

    sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
#endif
    cURLpp::terminate();
}

curlRequest::curlRequest(std::string url) {
    easyHandle.setOpt(cURLpp::Options::Url(url));
    // not sure how to use this when enabled
    easyHandle.setOpt(cURLpp::Options::SslVerifyHost(0L));
    // not sure how to use this when enabled
    easyHandle.setOpt(cURLpp::Options::SslVerifyPeer(0L));
    // Set SSL VERSION to TLS 1.2
    easyHandle.setOpt(cURLpp::Options::SslVersion(CURL_SSLVERSION_TLSv1_2));
    // Set timeout for the connection to build
    easyHandle.setOpt(cURLpp::Options::ConnectTimeout(10L));
    // Follow redirects (?)
    easyHandle.setOpt(cURLpp::Options::FollowLocation(1L));
    easyHandle.setOpt(cURLpp::Options::NoProgress(1L));
    // The function that will be used to write the data
    const auto writeBody = [this](char *ptr, size_t size, size_t nmemb) {
        size_t i;
        responseBody.reserve(nmemb * size);
        for (i = 0; i < size * nmemb; i++) {
            responseBody.push_back(ptr[i]/* == '\r' ? '\n' : ptr[i]*/);
        }

        return i;
    };
    easyHandle.setOpt(cURLpp::Options::WriteFunction(writeBody));
    // write function of response headers
    const auto writeHeaders = [this](char *ptr, size_t size, size_t nmemb) {
        size_t i;
        std::string headerString;
        headerString.reserve(nmemb * size);
        for (i = 0; i < size * nmemb; i++) {
            headerString.push_back(ptr[i] == '\r' ? '\n' : ptr[i]);
        }

        while (headerString.size() > 0) {
            long lineBreakIndex = headerString.find("\n");

            auto headerLine = headerString.substr(0, lineBreakIndex);
            long headerSep = headerLine.find(":");

            if (headerSep != std::string::npos) {
                auto header_value = headerLine.substr(headerSep + 1);
                header_value = header_value.substr(header_value.find_first_not_of(' '));
                headerMap[headerLine.substr(0, headerSep)] = header_value;
            }
            headerString = headerString.substr(lineBreakIndex + 1);
        }

        return i;
    };
    easyHandle.setOpt(cURLpp::Options::HeaderFunction(writeHeaders));
}

curlResponse curlRequest::performRequest() {
    easyHandle.perform();
    long responseCode = cURLpp::infos::ResponseCode::get(easyHandle);
    return curlResponse{responseBody, headerMap, responseCode};
}

void curlGetRequest::init() {}

curlGetRequest::curlGetRequest(const std::string &url) : curlRequest(url) {
    easyHandle.setOpt(cURLpp::Options::HttpGet(true));
}

void curlPostRequest::init() {}

curlPostRequest::curlPostRequest(const std::string &url, const std::string &bodyContent) : curlRequest(url) {
    easyHandle.setOpt(cURLpp::Options::PostFields(bodyContent));
    easyHandle.setOpt(cURLpp::Options::PostFieldSize(bodyContent.length()));
}


curlResponse curlDownloadFile(std::string url) {
    return curlGetRequest(url).performRequest();
}

curlResponse curlSendFile(std::string url, std::string bodyContent) {
    return curlPostRequest(url, bodyContent).performRequest();
}