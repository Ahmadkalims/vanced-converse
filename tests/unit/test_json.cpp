#include "../../src/common/SimpleJson.hpp"
#include <cassert>
#include <iostream>

void test_json_parsing() {
    std::string sample = R"({
        // Single line comment at the top
        "name": "YT Overlay", /* inline comment */
        "version": 1.5,
        "active": true, // trailing comment
        "tags": [
            "live",
            /* multi-line comment block
               with multiple lines */
            "stream",
            "chat"
        ],
        "nested": {
            // Comment inside object
            "interval": 15,
            "opacity": 45
        }
    })";

    json::Value root = json::parse(sample);
    assert(root.is_object());
    assert(root["name"].as_string() == "YT Overlay");
    assert(root["version"].as_double() == 1.5);
    assert(root["active"].as_bool() == true);
    assert(root["tags"].is_array());
    assert(root["tags"].size() == 3);
    assert(root["tags"][0].as_string() == "live");
    assert(root["tags"][1].as_string() == "stream");
    assert(root["tags"][2].as_string() == "chat");
    assert(root["nested"]["interval"].as_int() == 15);
    assert(root["nested"]["opacity"].as_int() == 45);

    // Serialization test
    std::string dumped = root.dump();
    json::Value re_parsed = json::parse(dumped);
    assert(re_parsed["name"].as_string() == "YT Overlay");
    assert(re_parsed["nested"]["interval"].as_int() == 15);

    std::cout << "[PASS] JSON Parser & Serializer Unit Tests (with Comments Support)" << std::endl;
}
