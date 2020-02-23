#ifndef ES_HTTP_HEADER_H
#define ES_HTTP_HEADER_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ES {

class HttpHeader : public ESB::EmbeddedListElement {
 public:
  HttpHeader(const unsigned char *fieldName, const unsigned char *fieldValue);

  virtual ~HttpHeader();

  /**
   * Get the field name.  This will always be present in a parsed header.
   *
   * @return the field name
   */
  inline const unsigned char *getFieldName() const { return _fieldName; }

  /**
   * Get the field value, if present.
   *
   * @return the field value or NULL if not set
   */
  inline const unsigned char *getFieldValue() const { return _fieldValue; }

  /** Set the field name.  Caller controls the memory for this value.
   *
   * @param name The field name.
   */
  inline void setFieldName(const unsigned char *fieldName) {
    _fieldName = fieldName;
  }

  /**
   * Set the field value.  Caller controls the memory for this value.
   *
   * @param value the field value or NULL if not set
   */
  inline void setFieldValue(const unsigned char *fieldValue) {
    _fieldValue = fieldValue;
  }

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *getCleanupHandler();

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESB::Allocator *allocator) {
    return allocator->allocate(size);
  }

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param block The object's memory.
   *  @return the block
   */
  inline void *operator new(size_t size, void *block) { return block; }

 private:
  // Disabled
  HttpHeader(const HttpHeader &);
  void operator=(const HttpHeader &);

  const unsigned char *_fieldName;
  const unsigned char *_fieldValue;
};

}  // namespace ES

#endif
