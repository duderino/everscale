#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#include <yajl/yajl_parse.h>

namespace ESB {

static int OnMapStart(void *ctx) { return ((AST::Callbacks *)ctx)->onMapStart(); }

static int OnMapKey(void *ctx, const unsigned char *key, size_t stringLen) {
  return ((AST::Callbacks *)ctx)->onMapKey(key, stringLen);
}

static int OnMapEnd(void *ctx) { return ((AST::Callbacks *)ctx)->onMapEnd(); }

static int OnListStart(void *ctx) { return ((AST::Callbacks *)ctx)->onListStart(); }

static int OnListEnd(void *ctx) { return ((AST::Callbacks *)ctx)->onListEnd(); }

static int OnNull(void *ctx) { return ((AST::Callbacks *)ctx)->onNull(); }

static int OnBoolean(void *ctx, int boolVal) { return ((AST::Callbacks *)ctx)->onBoolean(boolVal); }

static int OnInteger(void *ctx, long long int integerVal) { return ((AST::Callbacks *)ctx)->onInteger(integerVal); }

static int OnDouble(void *ctx, double doubleVal) { return ((AST::Callbacks *)ctx)->onDouble(doubleVal); }

static int OnString(void *ctx, const unsigned char *stringVal, size_t stringLen) {
  return ((AST::Callbacks *)ctx)->onString(stringVal, stringLen);
}

class StaticState {
 public:
  StaticState() {
    memset(&_callbacks, 0, sizeof(_callbacks));
    _callbacks.yajl_null = OnNull;
    _callbacks.yajl_boolean = OnBoolean;
    _callbacks.yajl_integer = OnInteger;
    _callbacks.yajl_double = OnDouble;
    _callbacks.yajl_string = OnString;
    _callbacks.yajl_start_map = OnMapStart;
    _callbacks.yajl_map_key = OnMapKey;
    _callbacks.yajl_end_map = OnMapEnd;
    _callbacks.yajl_start_array = OnListStart;
    _callbacks.yajl_end_array = OnListEnd;
  }

  inline yajl_callbacks &callbacks() { return _callbacks; }

 private:
  yajl_callbacks _callbacks;

  ESB_DEFAULT_FUNCS(StaticState);
};

static StaticState StaticState;

static void *AllocatorAlloc(void *ctx, size_t sz) {
  Allocator *allocator = (Allocator *)ctx;
  assert(allocator);
  void *block = NULL;
  return ESB_SUCCESS == allocator->allocate(sz, (void **)&block) ? block : NULL;
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
  void *block = NULL;
  return ESB_SUCCESS == allocator->reallocate(ptr, sz, (void **)&block) ? block : NULL;
}

JsonParser::JsonParser(AST::Callbacks &callbacks, Allocator &allocator) : _parser(NULL), _callbacks(callbacks) {
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
    _parser = yajl_alloc(&StaticState.callbacks(), alloc, &_callbacks);
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
