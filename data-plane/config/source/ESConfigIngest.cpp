#ifndef ES_CONFIG_INGEST_H
#include <ESConfigIngest.h>
#endif

#ifndef ESB_BUFFERED_FILE_H
#include <ESBBufferedFile.h>
#endif

#ifndef ESB_JSON_PARSER_H
#include <ESBJsonParser.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

#ifndef ESB_LOGGER_H
#include <ESBLogger.h>
#endif

namespace ES {

ConfigIngest::ConfigIngest(ESB::Allocator &allocator) : _allocator(allocator) {}

ConfigIngest::~ConfigIngest() {}

ESB::Error ConfigIngest::ingest(const char *path) {
  if (!path) {
    return ESB_NULL_POINTER;
  }

  ESB::AST::Tree tree(_allocator);
  ESB::Error error = parse(path, tree);
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannot parse/validate config file '%s'", path);
    return error;
  }

  error = process(tree);
  if (ESB_SUCCESS != error) {
    ESB_LOG_ERROR_ERRNO(error, "Cannpt process config file '%s'", path);
    return error;
  }

  return ESB_SUCCESS;
}

ESB::Error ConfigIngest::parse(const char *path, ESB::AST::Tree &tree) {
  assert(path);

  ESB::BufferedFile file(path, ESB::BufferedFile::READ_ONLY);
  unsigned char buffer[1024];
  ESB::JsonParser parser(tree, _allocator);

  while (true) {
    ESB::Size bytesRead = 0;
    switch (ESB::Error error = file.read(buffer, sizeof(buffer), &bytesRead)) {
      case ESB_SUCCESS:
        error = parser.parse(buffer, bytesRead);
        if (ESB_SUCCESS != error) {
          return error;
        }
        break;
      case ESB_BREAK:
        if (0 < bytesRead) {
          error = parser.parse(buffer, bytesRead);
          if (ESB_SUCCESS != error) {
            return error;
          }
        }
        return parser.end();
      default:
        return error;
    }
  }
}

ESB::Error ConfigIngest::process(const ESB::AST::Tree &tree) { return ESB_SUCCESS; }

}  // namespace ES
