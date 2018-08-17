#include <node.h>
#include "jsonify.hpp"

using namespace v8;

void InitAll(Local<Object> exports) {
    Jsonify::Init(exports);
}

NODE_MODULE(jsonify, InitAll)
