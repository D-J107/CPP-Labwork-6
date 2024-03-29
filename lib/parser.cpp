#include "parser.h"

std::string SliceTheString(std::string& s) {
    size_t pos1 = s.find_first_not_of("' '\t");
    if (pos1 != std::string::npos) {
        s = s.substr(pos1);
    }
    size_t pos2 = s.find_last_not_of("' '\t");
    if (pos2 != std::string::npos) {
        s = s.substr(0, pos2 + 1);
    }
    return s;
}

bool CheckInteger(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    char a = s[0];
    if (!(a == '-' || a == '+' || (a >= 48 && a <= 58))) {
        return false;
    }
    if (s.size() == 1 && (a == '-' || a == '+')) {
        return false;
    }
    for (int i = 1; i < s.size(); i++) {
        if (s[i] < 48 || s[i] > 58) { // 48-58 ascii codes <=> 0123456789
            return false;
        }
    }
    return true;
}

bool CheckFloat(const std::string& s) {
    bool dot_founded = false;
    char a = s[0];
    if (s.empty() || a == '.') {
        return false;
    }
    if (a == '-' || a == '+') {
        if (s.size() == 1) {
            return false;
        }
        if (s[1] == '.') {
            return false;
        }
    } else if (a < 48 || a > 58) {
        return false;
    }
    for (int i = 1; i < s.size(); i++) {
        if (s[i] == '.') {
            if (dot_founded || i == s.size() - 1) {
                return false;
            }
            dot_founded = true;
        } else if (s[i] < 48 || s[i] > 58) {
            return false;
        }
    }
    if (!dot_founded) {
        return false;
    }
    return true;
}

bool CheckString(const std::string& s) {
    if (!(s[0] == '"' && s.back() == '"')) {
        return false;
    }
    for (int i = 1; i < s.size() - 1; i++) {
        if (s[i] == '"' || s[i] == '\f' || s[i] == '\n' || s[i] == '\r') {
            return false;
        }
    }
    return true;
}

bool CheckBool(const std::string& s) {
    return (s == "true" || s == "false");
}

bool CheckArray(const std::string&s ) {
    if (!(s[0] == '[' && s.back() == ']')) {
        return false;
    }
    if (s.size() == 2) {
        return true;
    }
    uint8_t condition = 0;
    /*
     * 0 - wait for value
     * 1 - value is string
     * 2 - value is another array
     * 3 - value is int/float/flag
     * 4 - waiting for coma to separate values or ']' to end array
     */
    std::string buffer;
    std::pair<int,int> values_positions;
    values_positions.first = -1;
    values_positions.second = -1;
    std::string stack;
    for (int i = 1; i < s.size() - 1; i++) {
        if (condition == 0) {
            if (s[i] == ' ' || s[i] == '\t') {
                continue;
            }
            buffer += s[i];
            if (s[i] == '\"') {
                condition = 1;
                continue;
            }
            if (s[i] == '[') {
                stack += '[';
                values_positions.first = i;
                condition = 2;
                continue;
            }
            if (s[i] == ',') {
                return false;
            } else {
                condition = 3;
            }
        } else if (condition == 1) {
            buffer += s[i];
            if (i == s.size() - 1) { // String not finished
                return false;
            }
            if (s[i] == '\"') { // Reached end of the string
                if (!CheckString(buffer)) {
                    return false;
                } else {
                    buffer = "";
                    condition = 4;
                }
            }
        } else if (condition == 2) {
            if (s[i] == '\"') {
                if (stack.empty() || stack.back() != '\"') {
                    stack += s[i];
                } else if (stack.back() == '\"') {
                    stack.pop_back();
                }
            } else if (s[i] == '[' && stack.back() != '\"') {
                stack += s[i];
            } else if (s[i] == ']' && stack.back() == '[') {
                stack.pop_back();
                values_positions.second = i;
            }
            if (stack.empty()) {
                buffer = "";
                if (values_positions.first == -1 || values_positions.second == -1) {
                    return false;
                }
                std::string sub_array = s.substr(values_positions.first, values_positions.second - values_positions.first + 1);
                if (!CheckArray(sub_array)) {
                    return false;
                }
                values_positions.first = -1;
                values_positions.second = -1;
                condition = 4;
            } else if (i == s.size() - 2) {
                return false;
            }
        } else if (condition == 3) {
            if (i == s.size() - 2 || s[i] == ',') {
                std::string current_value = SliceTheString(buffer);
                if (!(CheckInteger(current_value) || CheckBool(current_value) || CheckFloat(current_value))) {
                    return false;
                }
                buffer = "";
                condition = 0;
                continue;
            }
            buffer += s[i];
        } else if (condition == 4) {
            if (s[i] == ' ' || s[i] == '\t') {
                continue;
            }
            if (s[i] != ',') {
                return false;
            }
            condition = 0;
        }
    }
    return true;
}

bool CheckKey(const std::string& s) {
    if (s.empty()) {
        return false;
    }
    for (char c : s) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_')) {
            return false;
        }
    }
    return true;
}

bool CheckSection(const std::string& s) {
    if (s.size() < 3 || s[0] != '[' || s.back() != ']') {
        return false;
    }
    std::string buffer;
    for (int i = 1; i < s.size() - 1; i++) {
        if (s[i] == '.' || i == s.size() - 2) {
            if (i == s.size() - 2) {
                buffer += s[i];
            }
            if(!CheckKey(buffer)) {
                return false;
            }
            buffer = "";
        } else {
            buffer += s[i];
        }
    }
    return true;
}

void omfl::parse(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Error! no file to parse" << '\n';
        return;
    }
    std::string argument = argv[1];
    std::filesystem::path path;
    if (argument == "--help" || argument == "-h") {
        std::cout << "OMFLParser program." << '\n';
        std::cout << "to enter data from file use this argument: --input=....config.omfl or -i=....config.omfl" << '\n';
        std::cout << "to see data from section named <sectionA> use command \"sectionA\"" << '\n';
        std::cout << "to see exact value from section <sectionA> from key named <keyA> use command \"sectionA.keyA\"" << '\n';
        std::cout << "to see exact value from key type array named <keyA> use command \"keyA[x]\"" << '\n';
    }
    if (argument.substr(0,8) == "--input=") {
        path = argument.substr(8);
    } else if (argument.substr(0,3) == "-i=") {
        path = argument.substr(3);
    } else {
        std::cout << "Wrong input command! input commant must be <--input=...> or <-i=...>" << '\n';
        return;
    }
    omfl::OMFLParser MyParser = omfl::parse(path);
    if (!MyParser.is_valid_format) {
        std::cout << "Error! some data parsed wrong" << '\n';
        return;
    }
    for (int i = 2; i < argc; i++) {
        std::string current_argument = static_cast<std::string>(argv[i]);
        std::string buffer;
        bool bracket = false;
        omfl::Section current_value = MyParser.global_section;
        if (MyParser.global_section.sections.find(current_argument) != MyParser.global_section.sections.end()) {
            MyParser.See(MyParser.global_section.sections[current_argument]);
        }
        for (int j = 0; j < current_argument.size(); j++) {
            if (current_argument[j] == '.' || (!bracket && j == current_argument.size() - 1) || current_argument[j] == '[') {
                if ((!bracket && j == current_argument.size() - 1)) {
                    buffer += current_argument[j];
                }
                current_value = current_value.Get(buffer);
                buffer = "";
                if (current_argument[j] == '[') {
                    bracket = true;
                }
            } else if (current_argument[j] == ']') {
                current_value = current_value[std::stoi(buffer)];
                bracket = false;
                buffer = "";
            } else {
                buffer += current_argument[j];
            }
        }
        if (current_value.IsString()) {
            std::cout << current_value.AsString() << '\n';
        }
        if (current_value.IsInt()) {
            std::cout << current_value.AsInt() << '\n';
        }
        if (current_value.IsFloat()) {
            std::cout << current_value.AsFloat() << '\n';
        }
        if (current_value.IsBool()) {
            std::cout << current_value.AsBool() << '\n';
        }
    }
}

omfl::OMFLParser omfl::parse(const std::filesystem::path& path) {
    std::ifstream fin;
    fin.open(path, std::ios::in);
    if (!fin) {
        std::cout << "Error! file cannot be opened" << '\n';
    }
    std::string file_name;
    for (char c : path.filename().generic_string()) {
        if (c == '.') {
            break;
        }
        file_name += c;
    }
    std::stringstream buffer_for_file;
    buffer_for_file << fin.rdbuf();
    return parse(buffer_for_file.str());
}

omfl::OMFLParser omfl::parse(const std::string& str) {
    OMFLParser MyParser;
    std::string buffer;
    std::string current_key;
    uint8_t condition = 0;

    // 0 - waiting for next data
    // 1 - # - comment
    // 2 - [ - section
    // 3 - key
    // 4 - value for key

    for (int i = 0; i < str.size(); i++) {
        char c = str[i];
        if (condition == 0) {
            if (c == '\n' || c == ' ' || c == '\t') {
                continue;
            }
            if (c == '#') {
                condition = 1;
                continue;
            }
            if (c == '[') {
                buffer += c;
                condition = 2;
                continue;
            } else {
                buffer += c;
                condition = 3;
                continue;
            }
        } else if (condition == 1) {
            if (c == '\n') {
                condition = 0;
            }
        } else if (condition == 2) {
            if (c == '\n') {
                std::cout << "Error! section name missed!" << '\n';
                MyParser.is_valid_format = false;
                return MyParser;
            }
            buffer += c;
            if (c == ']' || i == str.size() - 1) {
                if (!CheckSection(buffer)) {
                    std::cout << "Error! section " << buffer << " invalid!" << '\n';
                    MyParser.is_valid_format = false;
                    return MyParser;
                }
                std::string buffer_for_sections;
                MyParser.pointer_to_section = &MyParser.global_section;
                for (int j = 1; j < buffer.size(); j++) {
                    if (buffer[j] == '.' || j == buffer.size() - 1) {
                        if (buffer.size() == 3) {
                            buffer_for_sections = buffer[1];
                        }
                        if (MyParser.pointer_to_section->sections.find(buffer_for_sections) == MyParser.pointer_to_section->sections.end()) {
                            omfl::Section cur_section = Section(buffer_for_sections);
                            MyParser.pointer_to_section->sections[buffer_for_sections] = cur_section;
                        }
                        MyParser.pointer_to_section = &MyParser.pointer_to_section->sections[buffer_for_sections];
                        buffer_for_sections = "";
                        continue;
                    }
                    buffer_for_sections += buffer[j];
                }
                buffer = "";
                condition = 0;
            }
        } else if (condition == 3) {
            if (c == '\n' || buffer == "=" || i == str.size() - 1) {
                std::cout << "Error! key missed" << '\n';
                MyParser.is_valid_format = false;
                return MyParser;
            }
            if (c == '=') {
                std::string maybe_key = SliceTheString(buffer);
                if (!CheckKey(maybe_key)) {
                    std::cout << "Error! key name " << maybe_key << " invalid!" << '\n';
                    MyParser.is_valid_format = false;
                    return MyParser;
                }
                current_key = maybe_key;
                condition = 4;
                buffer = "";
            } else {
                buffer += c;
            }
        } else if (condition == 4) {
            if (c == '\n' || i == str.size() - 1 || (c == '#' && buffer[0] != '\"')) {
                if (c != '\n' && (c != '#' || buffer[0] == '\"')) {
                    buffer += c;
                }
                std::string current_value = SliceTheString(buffer);
                if (!(CheckInteger(current_value) || CheckFloat(current_value) || CheckString(current_value) || CheckBool(current_value) ||
                      CheckArray(current_value))) {
                    std::cout << "Error! value " << current_value << " invalid!" << '\n';
                    MyParser.is_valid_format = false;
                    return MyParser;
                }
                if (MyParser.pointer_to_section->values.find(current_key) == MyParser.pointer_to_section->values.end()) {
                    MyParser.pointer_to_section->values[current_key] = current_value;
                    buffer = "";
                    if (c == '#') {
                        condition = 1;
                    } else {
                        condition = 0;
                    }
                } else {
                    std::cout << "Error! redefinition" << current_key << '\n';
                    MyParser.is_valid_format = false;
                    return MyParser;
                }
            } else {
                buffer += c;
            }
        }
    }
    MyParser.pointer_to_section = &MyParser.global_section;
    return MyParser;
}

bool omfl::OMFLParser::valid() const {
    return OMFLParser::is_valid_format;
}

omfl::Section omfl::OMFLParser::Get(const std::string& str) const {
    return this->global_section.Get(str);
}

omfl::Section omfl::Section::Get(const std::string& str) const {
    Section current_section = *this;
    std::string buffer;
    for (int i = 0; i < str.size(); i++) {
        if (str[i] == '.' || i == str.size() - 1) {
            if (i == str.size() - 1) buffer += str[i];
            if (current_section.sections.find(buffer) != current_section.sections.end()) {
                omfl::Section section_for_copy = current_section.sections[buffer];
                current_section = section_for_copy;
            }
            if (i == str.size() - 1) {
                buffer.pop_back();
            }
        }
        buffer += str[i];
        if (str[i] == '.') {
            buffer = "";
        }
    }
    if (current_section.values.find(buffer) != current_section.values.end()) {
        current_section.row_answer = current_section.values[buffer];
        current_section.help_value = current_section.row_answer;
    }
    return current_section;
}

bool omfl::Section::IsInt() const {
    return CheckInteger(help_value);
}

bool omfl::Section::IsFloat() const {
    return CheckFloat(help_value);
}

bool omfl::Section::IsString() const {
    return CheckString(help_value);
}

bool omfl::Section::IsBool() const {
    return CheckBool(help_value);
}

bool omfl::Section::IsArray() const {
    return CheckArray(help_value);
}

int omfl::Section::AsInt() const {
    return std::stoi(help_value);
}

float omfl::Section::AsFloat() const {
    return std::stof(help_value);
}

std::string omfl::Section::AsString() const {
    std::string str = help_value;
    str.erase(0,1);
    str.pop_back();
    return str;
}

bool omfl::Section::AsBool() const {
    return (help_value == "true");
}

int omfl::Section::AsIntOrDefault(const int& value) const {
    if (CheckInteger(help_value)) {
        return std::stoi(help_value);
    }
    return value;
}

float omfl::Section::AsFloatOrDefault(const float& value) const {
    if (CheckFloat(help_value)) {
        return std::stof(help_value);
    }
    return value;
}

std::string omfl::Section::AsStringOrDefault(const std::string& value) const {
    if (CheckString(help_value)) {
        std::string answer = help_value;
        answer.erase(0,1);
        answer.pop_back();
        return answer;
    }
    return value;
}

omfl::Section& omfl::Section::operator[](const int& index) {
    uint32_t pos1 = 1;
    uint32_t pos2 = help_value.size() - 2;
    int counter = 0;
    std::string stack;
    for (int i = 1; i < help_value.size() - 1; i++) {
        if (help_value[i] == '\"') {
            if (!stack.empty() && stack.back() == '\"') {
                stack.pop_back();
            } else {
                stack += '\"';
            }
        }
        if (help_value[i] == '[' && (stack.empty() || (!stack.empty() && stack.back() != '\"'))) {
            stack += '[';
        }
        if (help_value[i] == ']' && !stack.empty() && stack.back() == '[') {
            stack.pop_back();
        }
        if (stack.empty() && help_value[i] == ',') {
            counter++;
            if (counter == index) {
                pos1 = i + 1;
            }
            if (counter == index + 1) {
                pos2 = i - 1;
            }
            if (i == help_value.size() - 2) {
                pos2 = i;
            }
        }
    }
    std::string help_string = help_value.substr(pos1,pos2 - pos1 + 1);
    help_value = SliceTheString(help_string);
    return *this;
}

void omfl::OMFLParser::See(const Section& section) {
    std::cout << '[' << section.name << "] ";
    if (section.sections.empty() && section.values.empty()) {
        std::cout << "section " << section.name << " is empty." << '\n';
    }
    if (!section.sections.empty()) {
        std::cout << "sub sections: ";
        for (auto& subsection : section.sections) {
            std::cout << '[' << subsection.first << "] ";
        }
        std::cout << '\n';
    }
    else {
        std::cout << "no subsections" << '\n';
    }
    if (!section.values.empty()) {
        std::cout << "keys: ";
        for (auto& key : section.values) {
            std::cout << key.first << ", ";
        }
        std::cout << '\n';
    }
    else {
        std::cout << "no keys" << '\n';
    }
}