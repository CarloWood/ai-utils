// The following header files (and their respective .cxx implementation)
// have been moved to https://github.com/CarloWood/memory
//
// Add that submodule to the root of your project:
//
// $ git submodule add https://github.com/CarloWood/memory.git
//
// Make sure to pull the latest commits from the master branch of cwm4.
// For example by running `./autogen.sh` in the root of the project, or
// by running:
//
// $ git submodule update --init --recursive
//
// in the root of your project.
//
// Change all includes for the following header files from using `utils/` to:
//
// #include "memory/DequeAllocator.h"
// #include "memory/DequeMemoryResource.h"
// #include "memory/MemoryPagePool.h"
// #include "memory/NodeMemoryPool.h"
// #include "memory/NodeMemoryResource.h"
// #include "memory/SimpleSegregatedStorage.h"
//
// Change all usage of namespace utils into memory for the following classes:
//
// memory::DequeAllocator
// memory::DequeMemoryResource
// memory::MemoryPagePool
// memory::NodeMemoryPool
// memory::Allocator
// memory::NodeMemoryResource
// memory::SimpleSegregatedStorage
//

#error "This header was moved to git submodule 'memory'. See the comment in the file containing this error for more information."
