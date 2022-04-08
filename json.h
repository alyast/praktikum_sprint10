#pragma once

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
	Node() = default;
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(double value);
    Node(std::string value);
    Node(std::nullptr_t);
    Node(bool value);

    const Array& AsArray() const;
    const Dict& AsMap() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;
    bool AsBool() const;

    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsNull() const;

private:
    /*
    Array as_array_;
    Dict as_map_;
    int as_int_ = 0;
    std::string as_string_;
    */
    std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> value_;
};

class Document {
public:
    explicit Document(Node root);
    const Node& GetRoot() const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

Node LoadString(std::istream& input);

bool operator== (const Node& lhs, const Node& rhs);
bool operator!= (const Node& lhs, const Node& rhs);

bool operator== (const Document& lhs, const Document& rhs);
bool operator!= (const Document& lhs, const Document& rhs);
}  // namespace json