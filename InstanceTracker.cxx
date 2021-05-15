#include "sys.h"
#include "InstanceTracker.h"
#include "utils/threading/aithreadid.h"

namespace utils::InstanceCollections {

std::set<detail::InstanceCollectionTracker*>* g_collection;

#ifdef CW_DEBUG
std::thread::id g_id;
#endif

void add(detail::InstanceCollectionTracker* instance_collection)
{
  // All calls to add() should be happening before main() is reached (which is why we can't use aithreadid::in_main_thread).
  ASSERT(aithreadid::is_single_threaded(g_id));
  if (!g_collection)
    g_collection = NEW(std::set<detail::InstanceCollectionTracker*>);
  g_collection->insert(instance_collection);
}

void remove(detail::InstanceCollectionTracker* instance_collection)
{
  // All calls to remove() should be happening after leaving main().
  ASSERT(aithreadid::is_single_threaded(g_id));
  g_collection->erase(instance_collection);
  // When the last element was removed we can destruct ourselves.
  if (g_collection->empty())
  {
    Dout(dc::notice, "Destroying InstanceCollections::g_collection");
    delete g_collection;
  }
}

void dump()
{
  for (detail::InstanceCollectionTracker const* instance_collection : *g_collection)
    instance_collection->dump();
}

} // namespace utils::InstanceCollections

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
channel_ct tracker("TRACKER");
NAMESPACE_DEBUG_CHANNELS_END
#endif
