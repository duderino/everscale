#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#include <yajl/yajl_parse.h>

namespace ESB {

static int OnMapStart(void *ctx) { return ((JsonParser *)ctx)->onMapStart(); }

static int OnMapKey(void *ctx, const unsigned char *key, size_t stringLen) {
  return ((JsonParser *)ctx)->onMapKey(key, stringLen);
}

static int OnMapEnd(void *ctx) { return ((JsonParser *)ctx)->onMapEnd(); }

static int OnArrayStart(void *ctx) { return ((JsonParser *)ctx)->onArrayStart(); }

static int OnArrayEnd(void *ctx) { return ((JsonParser *)ctx)->onArrayEnd(); }

static int OnNull(void *ctx) { return ((JsonParser *)ctx)->onNull(); }

static int OnBoolean(void *ctx, int boolVal) { return ((JsonParser *)ctx)->onBoolean(boolVal); }

static int OnInteger(void *ctx, long long int integerVal) { return ((JsonParser *)ctx)->onInteger(integerVal); }

static int OnDouble(void *ctx, double doubleVal) { return ((JsonParser *)ctx)->onDouble(doubleVal); }

static int OnString(void *ctx, const unsigned char *stringVal, size_t stringLen) {
  return ((JsonParser *)ctx)->onString(stringVal, stringLen);
}

class JsonParserCallbacks {
 public:
  JsonParserCallbacks() {
    memset(&_callbacks, 0, sizeof(_callbacks));
    _callbacks.yajl_null = OnNull;
    _callbacks.yajl_boolean = OnBoolean;
    _callbacks.yajl_integer = OnInteger;
    _callbacks.yajl_double = OnDouble;
    _callbacks.yajl_string = OnString;
    _callbacks.yajl_start_map = OnMapStart;
    _callbacks.yajl_map_key = OnMapKey;
    _callbacks.yajl_end_map = OnMapEnd;
    _callbacks.yajl_start_array = OnArrayStart;
    _callbacks.yajl_end_array = OnArrayEnd;
  }

  inline yajl_callbacks &callbacks() { return _callbacks; }

 private:
  yajl_callbacks _callbacks;

  ESB_DISABLE_AUTO_COPY(JsonParserCallbacks);
};

static JsonParserCallbacks Callbacks;

static void *AllocatorAlloc(void *ctx, size_t sz) {
  Allocator *allocator = (Allocator *)ctx;
  assert(allocator);
  return allocator->allocate(sz);
}

static void AllocatorDealloc(void *ctx, void *ptr) {
  Allocator *allocator = (Allocator *)ctx;
  assert(allocator);
  allocator->deallocate(ptr);
}

static void *AllocatorRealloc(void *ctx, void *ptr, size_t sz) {
  Allocator *allocator = (Allocator *)ctx;
  assert(allocator);
  assert(allocator->reallocates());
  return allocator->reallocate(ptr, sz);
}

JsonParser::JsonParser(Allocator &allocator) : _parser(NULL) {
  assert(sizeof(yajl_alloc_funcs) == sizeof(_opaque));
  yajl_alloc_funcs *alloc = (yajl_alloc_funcs *)&_opaque;
  alloc->malloc = AllocatorAlloc;
  alloc->free = AllocatorDealloc;
  alloc->realloc = AllocatorRealloc;
  alloc->ctx = &allocator;
}

JsonParser::~JsonParser() {
  if (_parser) {
    yajl_free((yajl_handle)_parser);
    _parser = NULL;
  }
}

Error JsonParser::parse(const unsigned char *buffer, UInt64 size) {
  if (!_parser) {
    yajl_alloc_funcs *alloc = (yajl_alloc_funcs *)&_opaque;
    _parser = yajl_alloc(&Callbacks.callbacks(), alloc, this);
    if (!_parser) {
      return ESB_OUT_OF_MEMORY;
    }
  }

  switch (yajl_status status = yajl_parse((yajl_handle)_parser, buffer, size)) {
    case yajl_status_ok:
      return ESB_SUCCESS;
    case yajl_status_client_canceled:
      return ESB_BREAK;
    case yajl_status_error:
      return ESB_CANNOT_PARSE;
    default:
      return ESB_INVALID_STATE;
  }
}

Error JsonParser::end() {
  if (!_parser) {
    return ESB_INVALID_STATE;
  }

  switch (yajl_status status = yajl_complete_parse((yajl_handle)_parser)) {
    case yajl_status_ok:
      return ESB_SUCCESS;
    case yajl_status_client_canceled:
      return ESB_BREAK;
    case yajl_status_error:
      return ESB_CANNOT_PARSE;
    default:
      return ESB_INVALID_STATE;
  }
}

}  // namespace ESB
