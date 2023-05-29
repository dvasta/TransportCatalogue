#include "json_builder.h"

#include <iostream>

namespace json {
using namespace std::literals;

/* ---------------- Builder ---------------*/

Builder::KeyItemContext Builder::Key(const std::string& key) {
    CheckRoot();
    if (nodes_stack_.empty()) {
        throw std::logic_error("Incorrect call .Key()"s);
    }
    Dict& dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
    Node node;
    auto [iter,  success] = dict.emplace(key, node);
    nodes_stack_.emplace_back(&(*iter).second);
    return *this;
}

Builder& Builder::Value(const Node::Value& value) {
    CheckRoot();
    if (nodes_stack_.empty()) {
        const_cast<Node::Value&>(root_.GetValue()) = std::move(value);
    } else if (nodes_stack_.back()->IsArray()) {
        Node node;
        const_cast<Node::Value&>(node.GetValue()) = std::move(value);
        Array& array = const_cast<Array&>(nodes_stack_.back()->AsArray());
        array.emplace_back(node);
    } else if (nodes_stack_.back()->IsNull()) {
        const_cast<Node::Value&>(nodes_stack_.back()->GetValue()) = std::move(value);
        nodes_stack_.pop_back();
    } else {
        throw std::logic_error("Incorrect call .Value()"s);
    }
    return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
    CheckRoot();
    Array array;
    nodes_stack_.emplace_back(new Node(array));
    return *this;
}

Builder& Builder::EndArray() {
    CheckRoot();
    if (!nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Not Array. Can't call .EndArray()"s);
    }
    Node::Value value = std::move(nodes_stack_.back()->AsArray());
    nodes_stack_.pop_back();
    Value(value);
    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    CheckRoot();
    Dict dict;
    nodes_stack_.emplace_back(new Node(dict));
    return *this;
}

Builder& Builder::EndDict() {
    CheckRoot();
    Node::Value value = std::move(nodes_stack_.back()->AsDict());
    nodes_stack_.pop_back();
    Value(value);
    return *this;
}

Node Builder::Build() const {
    if (!nodes_stack_.empty()) {
         throw std::logic_error("Stack is not empty"s);
    }
    if (root_.IsNull()) {
        throw std::logic_error("Root is null"s);
    }
    return root_;
}

void Builder::CheckRoot() const {
    if (!root_.IsNull()) {
        throw std::logic_error("Root built"s);
    }
}

/* ------------ end Builder ---------------*/

/* ------------ BaseContext ---------------*/

Builder::BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

Builder::KeyItemContext Builder::BaseContext::Key(const std::string& key) {
    return builder_.Key(key);
}

Builder& Builder::BaseContext::Value(const Node::Value& value) {
    return builder_.Value(value);
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

/* ------------ ValueContext ---------------*/

Builder::DictItemContext Builder::KeyItemContext::Value(const Node::Value& value) {
    return BaseContext::Value(value);
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(const Node::Value& value) {
    return BaseContext::Value(value);
}

} //namespace json
