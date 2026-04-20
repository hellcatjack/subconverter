#include <iostream>
#include <string>
#include <vector>

#include "handler/settings.h"
#include "handler/webget.h"
#include "parser/subparser.h"
#include "utils/base64/base64.h"

int fileWrite(const std::string &, const std::string &, bool)
{
    return 0;
}

namespace
{
bool require(bool condition, const std::string &message)
{
    if(condition)
        return true;

    std::cerr << message << std::endl;
    return false;
}
}

int main()
{
    global.logLevel = LOG_LEVEL_FATAL;

    const std::string input =
        "vless://11111111-1111-4111-8111-111111111111@us.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyUS1111111111111111111111111&sid=1111aaaabbbb2222&type=xhttp&host=www.microsoft.com&path=%2Fexample-us-path&mode=stream-one&packetEncoding=xudp#TEST-US-REALITY\n"
        "vless://22222222-2222-4222-8222-222222222222@hk.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyHK2222222222222222222222222&sid=3333ccccdddd4444&type=xhttp&host=www.microsoft.com&path=%2Fexample-hk-path&mode=stream-one&packetEncoding=xudp#TEST-HK-REALITY\n"
        "vless://33333333-3333-4333-8333-333333333333@in.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyIN3333333333333333333333333&sid=5555eeeeffff6666&type=xhttp&host=www.microsoft.com&path=%2Fexample-in-path&mode=stream-one&packetEncoding=xudp#TEST-IN-REALITY";

    const std::string data_uri = "data:text/plain;base64," + base64Encode(input);
    const std::string fetched = webGet(data_uri);

    if(!require(fetched == input, "webGet(data:) did not return the original subscription content"))
    {
        std::cerr << "expected size: " << input.size() << ", actual size: " << fetched.size() << std::endl;
        return 1;
    }

    std::vector<Proxy> nodes;
    if(!require(explodeConfContent(fetched, nodes) == 1, "explodeConfContent(data uri content) returned no nodes"))
        return 2;

    return require(nodes.size() == 3, "expected 3 nodes after data uri decode, got " + std::to_string(nodes.size()))
        ? 0
        : 3;
}
