#pragma once

#include <string>
#include <optional>
#include <algorithm>

#include "json.h"

namespace json {

class Builder {
private:
    class BaseContext;
    struct KeyItemContext;
    struct ArrayItemContext;
    struct DictItemContext;

public:
    KeyItemContext Key(const std::string& key);
    Builder& Value(const Node::Value& value);
    ArrayItemContext StartArray();
    Builder& EndArray();
    DictItemContext StartDict();
    Builder& EndDict();

    Node Build() const;

private:
    Node root_;
    std::vector<Node*> nodes_stack_;

    void CheckRoot() const;
};

class Builder::BaseContext {
public:
    BaseContext(Builder& builder);
    KeyItemContext Key(const std::string& key);
    Builder& Value(const Node::Value& value);
    ArrayItemContext StartArray();
    Builder& EndArray();
    DictItemContext StartDict();
    Builder& EndDict();

private:
    Builder& builder_;
};

struct Builder::KeyItemContext final : private BaseContext {
    using BaseContext::BaseContext;
    DictItemContext Value(const Node::Value& value);
    using BaseContext::StartArray;
    using BaseContext::StartDict;
};

struct Builder::ArrayItemContext final : private BaseContext {
    using BaseContext::BaseContext;
    ArrayItemContext Value(const Node::Value& value);
    using BaseContext::StartArray;
    using BaseContext::EndArray;
    using BaseContext::StartDict;
};

struct Builder::DictItemContext final : private BaseContext {
    using BaseContext::BaseContext;
    using BaseContext::Key;
    using BaseContext::EndDict;
};

} // namespace json
