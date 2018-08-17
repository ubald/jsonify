#pragma once

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <list>
#include <iostream>
#include <utility>

class Jsonify : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  v8::Persistent<v8::Function> parse_cb;

private:
  Jsonify(v8::Local<v8::Function> parse_callback);
  ~Jsonify();

  static v8::Persistent<v8::Function> constructor;

  void release();

  // async log
  uv_async_t parse_async{};
  uv_mutex_t parse_mutex{};
  std::list<std::string> parse_list{};

  static v8::Handle<v8::Value> parseJson(v8::Handle<v8::Value> jsonString);

  static void NotifyParse(uv_async_s *async);
  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Parse(const v8::FunctionCallbackInfo<v8::Value> &args);
};
