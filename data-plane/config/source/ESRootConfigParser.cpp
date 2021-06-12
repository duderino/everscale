#ifndef ES_ROOT_CONFIG_PARSER_H
#include <ESRootConfigParser.h>
#endif

namespace ES {

RootConfigParser::RootConfigParser(ESB::Allocator &allocator) : JsonParser(allocator), _stack() {}

RootConfigParser::~RootConfigParser() {}

ESB::JsonParser::ParseControl RootConfigParser::onMapStart() { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onMapKey(const unsigned char *key, ESB::UInt32 length) {
  return CONTINUE;
}

ESB::JsonParser::ParseControl RootConfigParser::onMapEnd() { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onArrayStart() { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onArrayEnd() { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onNull() { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onBoolean(bool value) { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onInteger(ESB::Int64 value) { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onDouble(double value) { return CONTINUE; }

ESB::JsonParser::ParseControl RootConfigParser::onString(const unsigned char *value, ESB::UInt32 length) {
  return CONTINUE;
}

}  // namespace ES
