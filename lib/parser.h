#pragma once

#include <filesystem>
#include <istream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace omfl {
    struct Section {
        Section(std::string name = "") {
            this->name = name;
        }
        std::map<std::string,std::string> values;
        std::map<std::string,Section> sections;
        std::string row_answer;
        std::string help_value;
        std::string name;

        Section Get(const std::string& str) const;
        bool IsInt() const;
        bool IsFloat() const;
        bool IsString() const;
        bool IsBool() const;
        bool IsArray() const;

        int AsInt() const;
        float AsFloat() const;
        std::string AsString() const;
        bool AsBool() const;

        int AsIntOrDefault(const int& value) const;
        float AsFloatOrDefault(const float& value) const;
        std::string AsStringOrDefault(const std::string& value) const;

        Section& operator [](const int& index);
    };

    class OMFLParser {
    public:
        OMFLParser() {
            global_section = {"global_section"};
            pointer_to_section = &global_section;
        }
        Section Get(const std::string& str) const;
        bool valid() const;
        bool is_valid_format = true;
        Section global_section;
        Section* pointer_to_section;
    };
    OMFLParser parse(const std::filesystem::path& path);
    OMFLParser parse(const std::string& str);
    void parse(int argc, char** argv);
}