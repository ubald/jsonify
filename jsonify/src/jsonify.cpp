#include "./jsonify.hpp"

using namespace std;
using namespace v8;
using namespace node;

Persistent<Function> Jsonify::constructor;

Jsonify::Jsonify(Local<Function> parse_callback) {
  Isolate *isolate = Isolate::GetCurrent();
  parse_cb.Reset(isolate, parse_callback);
  uv_mutex_init(&parse_mutex);
  parse_async.data = this;
  uv_async_init(uv_default_loop(), &parse_async, NotifyParse);
};

Jsonify::~Jsonify(){};

void Jsonify::Init(Local<Object> exports) {
  Isolate *isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Jsonify"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "parse", Parse);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Jsonify"), tpl->GetFunction());
}

void Jsonify::release() {
  if (!uv_is_closing((uv_handle_t *)&parse_async)) {
    uv_close((uv_handle_t *)&parse_async, nullptr);
  }
  uv_mutex_destroy(&parse_mutex);
  parse_cb.Reset();
}

void Jsonify::New(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope scope(isolate);

  if (args.IsConstructCall()) {
    if (args.Length() < 1 || !args[0]->IsFunction()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate,
          "Wrong arguments. Jsonify requires a callback.")));
      return;
    }

    Jsonify *obj = new Jsonify(Local<Function>::Cast(args[0]));
    obj->Wrap(args.This());
    return args.GetReturnValue().Set(args.This());
  }
}

Handle<Value> Jsonify::parseJson(Handle<Value> jsonString) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  Handle<Object> global = isolate->GetCurrentContext()->Global();
  Handle<Object> JSON = global->Get(String::NewFromUtf8(isolate, "JSON"))->ToObject();
  Handle<Function> JSON_parse = Handle<Function>::Cast(JSON->Get(String::NewFromUtf8(isolate, "parse")));
  return JSON_parse->Call(JSON, 1, &jsonString);
}

void Jsonify::NotifyParse(uv_async_s *async) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  Jsonify *obj = static_cast<Jsonify *>(async->data);
  if (!obj->parse_cb.IsEmpty()) {
    Local<Function> cb = Local<Function>::New(isolate, obj->parse_cb);
    if (cb->IsCallable()) {
      TryCatch try_catch;
      for (auto &it : obj->parse_list) {
        Local<String> jsonStr = String::NewFromUtf8(isolate, it.c_str());
        Handle<Value> json = parseJson(jsonStr);
        Local<Value> argv[] = {json};
        cb->Call(isolate->GetCurrentContext()->Global(), 1, argv);
      }
      obj->parse_list.clear();
      if (try_catch.HasCaught()) {
        FatalException(isolate, try_catch);
      }
    }
  }
}

void Jsonify::Parse(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HandleScope scope(isolate);
  Jsonify *obj = ObjectWrap::Unwrap<Jsonify>(args.This());

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong number of arguments")));
    return;
  }
  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Argument must be a string")));
    return;
  }

  uv_mutex_lock(&obj->parse_mutex);
  v8::String::Utf8Value v8str(args[0]->ToString());
  std::string str(*v8str);
  obj->parse_list.push_back(str);
  uv_mutex_unlock(&obj->parse_mutex);
  uv_async_send(&obj->parse_async);
}
