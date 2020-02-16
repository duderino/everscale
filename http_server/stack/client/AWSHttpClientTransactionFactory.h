/* Copyright (c) 2009 Yahoo! Inc.  All rights reserved.
 * The copyrights embodied in the content of this file are licensed by Yahoo!
 * Inc. under the BSD (revised) open source license.
 */

#ifndef AWS_HTTP_CLIENT_TRANSACTION_FACTORY_H
#define AWS_HTTP_CLIENT_TRANSACTION_FACTORY_H

#ifndef ESF_EMBEDDED_LIST_H
#include <ESFEmbeddedList.h>
#endif

#ifndef ESF_MUTEX_H
#include <ESFMutex.h>
#endif

#ifndef ESF_DISCARD_ALLOCATOR_H
#include <ESFDiscardAllocator.h>
#endif

#ifndef ESF_LOGGER_H
#include <ESFLogger.h>
#endif

#ifndef AWS_HTTP_CLIENT_TRANSACTION_H
#include <AWSHttpClientTransaction.h>
#endif

#ifndef AWS_HTTP_CLIENT_HANDLER_H
#include <AWSHttpClientHandler.h>
#endif

/** A factory that creates and reuses AWSHttpClientTransactions
 */
class AWSHttpClientTransactionFactory {
 public:
  AWSHttpClientTransactionFactory();

  virtual ~AWSHttpClientTransactionFactory();

  ESFError initialize();

  void destroy();

  AWSHttpClientTransaction *create(AWSHttpClientHandler *clientHandler);

  void release(AWSHttpClientTransaction *transaction);

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, ESFAllocator *allocator) {
    return allocator->allocate(size);
  }

 private:
  // Disabled
  AWSHttpClientTransactionFactory(const AWSHttpClientTransactionFactory &);
  AWSHttpClientTransactionFactory &operator=(
      const AWSHttpClientTransactionFactory &);

  class CleanupHandler : public ESFCleanupHandler {
   public:
    /** Constructor
     */
    CleanupHandler(AWSHttpClientTransactionFactory *factory);

    /** Destructor
     */
    virtual ~CleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(ESFObject *object);

   private:
    // Disabled
    CleanupHandler(const CleanupHandler &);
    void operator=(const CleanupHandler &);

    AWSHttpClientTransactionFactory *_factory;
  };

  ESFLogger *_logger;
  ESFDiscardAllocator _allocator;
  ESFEmbeddedList _embeddedList;
  ESFMutex _mutex;
  CleanupHandler _cleanupHandler;
};

#endif
