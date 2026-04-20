#include <iostream>
#include <string>
#include <vector>

#include "generator/config/subexport.h"
#include "parser/subparser.h"

namespace
{
bool requireContains(const std::string &output, const std::string &needle)
{
    if(output.find(needle) != std::string::npos)
        return true;

    std::cerr << "missing expected field: " << needle << std::endl;
    return false;
}
}

int main()
{
    const std::string input =
        "vless://11111111-1111-4111-8111-111111111111@us.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyUS1111111111111111111111111&sid=1111aaaabbbb2222&type=xhttp&host=www.microsoft.com&path=%2Fexample-us-path&mode=stream-one&packetEncoding=xudp#TEST-US-REALITY\n"
        "vless://22222222-2222-4222-8222-222222222222@hk.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyHK2222222222222222222222222&sid=3333ccccdddd4444&type=xhttp&host=www.microsoft.com&path=%2Fexample-hk-path&mode=stream-one&packetEncoding=xudp#TEST-HK-REALITY\n"
        "vless://33333333-3333-4333-8333-333333333333@in.example.com:18443?encryption=none&security=reality&sni=www.microsoft.com&fp=chrome&pbk=examplePublicKeyIN3333333333333333333333333&sid=5555eeeeffff6666&type=xhttp&host=www.microsoft.com&path=%2Fexample-in-path&mode=stream-one&packetEncoding=xudp#TEST-IN-REALITY";

    std::vector<Proxy> nodes;
    explodeSub(input, nodes);
    if(nodes.size() != 3)
    {
        std::cerr << "expected 3 nodes, got " << nodes.size() << std::endl;
        return 1;
    }

    YAML::Node root;
    extra_settings ext;
    ext.clash_new_field_name = true;
    proxyToClash(nodes, root, ProxyGroupConfigs{}, false, ext);
    const std::string output = YAML::Dump(root);

    return requireContains(output, "type: vless")
        && requireContains(output, "name: TEST-US-REALITY")
        && requireContains(output, "name: TEST-HK-REALITY")
        && requireContains(output, "name: TEST-IN-REALITY")
        && requireContains(output, "server: us.example.com")
        && requireContains(output, "server: hk.example.com")
        && requireContains(output, "server: in.example.com")
        && requireContains(output, "udp: true")
        && requireContains(output, "tls: true")
        && requireContains(output, "servername: www.microsoft.com")
        && requireContains(output, "network: xhttp")
        && requireContains(output, "packet-encoding: xudp")
        && requireContains(output, "client-fingerprint: chrome")
        && requireContains(output, "xhttp-opts")
        && requireContains(output, "path: /example-us-path")
        && requireContains(output, "host: www.microsoft.com")
        && requireContains(output, "reality-opts")
        && requireContains(output, "public-key: examplePublicKeyUS1111111111111111111111111")
        && requireContains(output, "short-id: 1111aaaabbbb2222")
        && requireContains(output, "mode: stream-one")
        ? 0
        : 2;
}
