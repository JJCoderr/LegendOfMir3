#pragma once
#include <string>
#include <vector>
struct LoginConfiguration {
    struct Server {std::string caption,name;};
    std::string host,accountUrl,passwordUrl;
    WORD port=0;
    std::vector<Server> servers;
};
