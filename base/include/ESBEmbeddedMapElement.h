#ifndef ESB_EMBEDDED_MAP_ELEMENT_H
#define ESB_EMBEDDED_MAP_ELEMENT_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ESB {

/** An element that can be stored in an EmbeddedMap
 *
 *  @ingroup collection
 */
class EmbeddedMapElement : public EmbeddedListElement {
 public:
  /** Constructor.
   */
  EmbeddedMapElement();

  /** Destructor.
   */
  virtual ~EmbeddedMapElement();

  /** Extract the key from the element.
   *
   * @return The element's key.
   */
  virtual const void *key() const;

 private:
  // Disabled
  EmbeddedMapElement(const EmbeddedMapElement &);
  EmbeddedMapElement &operator=(const EmbeddedMapElement &);
};

}  // namespace ESB

#endif
