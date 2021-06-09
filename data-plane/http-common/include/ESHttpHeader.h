#ifndef ES_HTTP_HEADER_H
#define ES_HTTP_HEADER_H

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

namespace ES {

class HttpHeader : public ESB::EmbeddedListElement {
 public:
  HttpHeader(const char *fieldName, const char *fieldValue = NULL);
  HttpHeader(const unsigned char *fieldName, const unsigned char *fieldValue = NULL);
  virtual ~HttpHeader();

  /**
   * Get the field name.  This will always be present in a parsed header.
   *
   * @return the field name
   */
  inline const unsigned char *fieldName() const { return _fieldName; }

  /**
   * Get the field value, if present.
   *
   * @return the field value or NULL if not set
   */
  inline const unsigned char *fieldValue() const { return _fieldValue; }

  /** Set the field name.  Caller controls the memory for this value.
   *
   * @param name The field name.
   */
  inline void setFieldName(const char *fieldName) { _fieldName = (const unsigned char *)fieldName; }

  /** Set the field name.  Caller controls the memory for this value.
   *
   * @param name The field name.
   */
  inline void setFieldName(const unsigned char *fieldName) { _fieldName = fieldName; }

  /**
   * Set the field value.  Caller controls the memory for this value.
   *
   * @param value the field value or NULL if not set
   */
  inline void setFieldValue(const char *fieldValue) { _fieldValue = (const unsigned char *)fieldValue; }

  /**
   * Set the field value.  Caller controls the memory for this value.
   *
   * @param value the field value or NULL if not set
   */
  inline void setFieldValue(const unsigned char *fieldValue) { _fieldValue = fieldValue; }

  /** Return an optional handler that can destroy the element.
   *
   * @return A handler to destroy the element or NULL if the element should not
   * be destroyed.
   */
  virtual ESB::CleanupHandler *cleanupHandler();

 private:
  const unsigned char *_fieldName;
  const unsigned char *_fieldValue;

  ESB_DEFAULT_FUNCS(HttpHeader);
};

}  // namespace ES

#endif
