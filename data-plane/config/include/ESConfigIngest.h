#ifndef ES_CONFIG_INGEST_H
#define ES_CONFIG_INGEST_H

#ifndef ESB_COMMON_H
#include <ESBCommon.h>
#endif

#ifndef ESB_AST_TREE_H
#include <ASTTree.h>
#endif

namespace ES {

class ConfigIngest {
 public:
  ConfigIngest(ESB::Allocator &allocator);

  virtual ~ConfigIngest();

  /** Ingest a JSON config file
   *
   * @param path The path to the JSON config file
   * @return ESB_SUCCESS if successful, another error code otherwise.
   */
  ESB::Error ingest(const char *path);

 private:
  ESB::Error parse(const char *path, ESB::AST::Tree &tree);

  ESB::Error process(const ESB::AST::Tree &tree);

  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(ConfigIngest);
};

}  // namespace ES

#endif
