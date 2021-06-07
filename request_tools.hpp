//
// Created by kouta on 4/6/21.
//

#ifndef VITANET_REQUEST_TOOLS_HPP
#define VITANET_REQUEST_TOOLS_HPP

void netInit();
void netTerm();
void httpInit();
void httpTerm();

struct curlResponse {
    std::vector<char> body;
    std::map<std::string, std::string> headers;
    long responseCode;
};

class curlRequest {

    std::vector<char> responseBody;
    std::map<std::string, std::string> headerMap;

protected:
    cURLpp::Easy easyHandle;


public:
    curlRequest(std::string url);

    virtual curlResponse performRequest();
};

class curlGetRequest : public curlRequest {
protected:
    virtual void init();

public:
    curlGetRequest(const std::string &url);
};

class curlPostRequest : public curlRequest {
protected:
    virtual void init();;
public:
    curlPostRequest(const std::string &url, const std::string &bodyContent);
};


curlResponse curlSendFile(std::string url, std::string bodyContent);

curlResponse curlDownloadFile(std::string url);

#endif //VITANET_REQUEST_TOOLS_HPP
