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
  ESB::Error parse(const char *path);

 private:
  ESB::Error validate(const ESB::AST::Tree &config) const;
  ESB::Error validateTLSContext(const ESB::AST::Map &context) const;
  ESB::Error validateTLSIndex(const ESB::AST::Map &index) const;
  ESB::Error validateSecurityRule(const ESB::AST::Map &rule) const;
  ESB::Error validateLimit(const ESB::AST::Map &limit) const;
  ESB::Error validateCluster(const ESB::AST::Map &cluster) const;
  ESB::Error validatePort(const ESB::AST::Map &port) const;
  ESB::Error validateCIDRMap(const ESB::AST::Map &cidrMap) const;
  ESB::Error validateRuleList(const ESB::AST::Map &rules) const;
  ESB::Error validateExtension(const ESB::AST::Map &extension) const;
  ESB::Error validateRequest(const ESB::AST::Map &request) const;

  ESB::Allocator &_allocator;

  ESB_DEFAULT_FUNCS(ConfigIngest);
};

}  // namespace ES

#endif
