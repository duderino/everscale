/** @file ESFBoilerplate.h
 *  @brief The boilerplate for most header files
 *
 *  Copyright 2009 Joshua Blatt
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *    $Author: blattj $
 *    $Date: 2009/05/25 21:51:05 $
 *    $Name:  $
 *    $Revision: 1.3 $
 */

#ifndef ESF_BOILERPLATE_H
#define ESF_BOILERPLATE_H

#ifndef ESF_CONFIG_H
#include <ESFConfig.h>
#endif

#ifndef ESF_ALLOCATOR_H
#include <ESFAllocator.h>
#endif

/** @defgroup boilerplate Boilerplate
 */

/** The ESFBoilerplate Class
 *
 *  @ingroup boilerplate
 */
class ESFBoilerplate {
public:
    /** Constructor
     */
    ESFBoilerplate();

    /** Destructor.
     */
    virtual ~ESFBoilerplate();

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
    ESFBoilerplate(const ESFBoilerplate &);
    ESFBoilerplate &operator=(const ESFBoilerplate &);
};

#endif /* ! ESF_BOILERPLATE_H */
