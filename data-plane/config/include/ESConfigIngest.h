#ifndef ES_CONFIG_INGEST_H
#define ES_CONFIG_INGEST_H

#ifndef ESB_ROOT_CONFIG_PARSER_H
#include <ESRootConfigParser.h>
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
  ESB::Error parse(const char *path);

 private:
  ES::RootConfigParser _parser;
  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(ConfigIngest);
};

}  // namespace ES

#endif
