#ifndef ESB_COMMAND_H
#define ESB_COMMAND_H

#ifndef ESB_CONFIG_H
#include <ESBConfig.h>
#endif

#ifndef ESB_ALLOCATOR_H
#include <ESBAllocator.h>
#endif

#ifndef ESB_EMBEDDED_LIST_ELEMENT_H
#include <ESBEmbeddedListElement.h>
#endif

#ifndef ESB_SHARED_INT_H
#include <ESBSharedInt.h>
#endif

namespace ESB {

/** @defgroup thread Thread
 */

/** The interface for commands run by threadpools, etc.
 *
 *  @ingroup thread
 */
class Command : public EmbeddedListElement {
 public:
  /** Constructor
   *
   */
  Command();

  /** Destructor.
   */
  virtual ~Command();

  /** Get the name of the command.  This name can be used in logging messages,
   * etc.
   *
   * @return The command's name
   */
  virtual const char *name() const = 0;

  /** Run the command
   *
   * @param isRunning This object will return true as long as the controlling
   * thread isRunning, false when the controlling thread wants to shutdown.
   * @return If true, caller should destroy the command with the CleanupHandler.
   */
  virtual bool run(SharedInt *isRunning) = 0;

  /** Placement new.
   *
   *  @param size The size of the object.
   *  @param allocator The source of the object's memory.
   *  @return Memory for the new object or NULL if the memory allocation failed.
   */
  inline void *operator new(size_t size, Allocator &allocator) noexcept {
    return allocator.allocate(size);
  }

 private:
  // Disabled
  Command(const Command &);
  Command &operator=(const Command &);
};

}  // namespace ESB

#endif
