/** @file ESBAllocatorCleanupHandler.h
 *  @brief An object that can destroy another object that was created by an allocator
 */

#ifndef ESB_ALLOCATOR_CLEANUP_HANDLER_H
#define ESB_ALLOCATOR_CLEANUP_HANDLER_H

#ifndef ESB_CLEANUP_HANDLER_H
#include <ESBCleanupHandler.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

namespace ESB {
	
/** An object that can destroy another object that was created by an allocator
 *
 *  @ingroup collection
 */
class AllocatorCleanupHandler: public CleanupHandler {
public:

    /** Constructor
     *
     * @param allocator The allocator to free the object with
     */
    AllocatorCleanupHandler(Allocator *allocator);

    /** Destructor
     */
    virtual ~AllocatorCleanupHandler();

    /** Destroy an object
     *
     * @param object The object to destroy
     */
    virtual void destroy(Object *object);

private:
    // Disabled
    AllocatorCleanupHandler(const AllocatorCleanupHandler &);
    void operator=(const AllocatorCleanupHandler &);

    Allocator *_allocator;
};

}

#endif
