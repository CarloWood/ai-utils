/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class InstanceTracker.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "threadsafe/aithreadsafe.h"
#include "utils/print_using.h"
#include <set>
#include "debug.h"

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct tracker;
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {

namespace detail {
class InstanceCollectionTracker;
} // namespace detail

namespace InstanceCollections {

extern std::set<detail::InstanceCollectionTracker*>* g_collection;
#ifdef CW_DEBUG
extern std::thread::id g_id;
#endif

void add(detail::InstanceCollectionTracker* instance_collection);
void remove(detail::InstanceCollectionTracker* instance_collection);
#ifdef CW_DEBUG
void dump();
#endif

} // namespace InstanceCollections

namespace detail {

class InstanceCollectionTracker
{
 public:
  InstanceCollectionTracker()
  {
    InstanceCollections::add(this);
  }

  ~InstanceCollectionTracker()
  {
    InstanceCollections::remove(this);
  }

#ifdef CWDEBUG
  virtual void dump() const = 0;
#endif
};

template<typename T>
class InstanceCollection : public InstanceCollectionTracker
{
 public:
  using collection_t = aithreadsafe::Wrapper<std::set<T*>, aithreadsafe::policy::Primitive<std::mutex>>;

 private:
  collection_t m_collection;

 public:
  InstanceCollection() { }

  void add(T* instance)
  {
    typename collection_t::wat collection_w(m_collection);
    collection_w->insert(instance);
  }

  void remove(T* instance)
  {
    typename collection_t::wat collection_w(m_collection);
    collection_w->erase(instance);
  }

  void for_each_instance(std::function<void(T const*)> const& func) const
  {
    typename collection_t::crat collection_r(m_collection);
    for (T const* instance : *collection_r)
      func(instance);
  }

 private:
#ifdef CWDEBUG
  // Implementation of base class interface.
  void dump() const override
  {
    Dout(dc::tracker, "Instances of " << type_info_of<T>().demangled_name() << ":");
    debug::Indent indent(2);
    for_each_instance([&](T const* instance)
    {
      Dout(dc::tracker, utils::print_using(*instance, &T::print_tracker_info_on));
    });
  }
#endif
};

} // namespace detail

template<typename T>
class InstanceTracker
{
 private:
  static detail::InstanceCollection<T> s_collection;

 protected:
  InstanceTracker()
  {
    s_collection.add(static_cast<T*>(this));
  }

  ~InstanceTracker()
  {
    s_collection.remove(static_cast<T*>(this));
  }

 public:
  static void for_each_instance(std::function<void(T const*)> const& func)
  {
    s_collection.for_each_instance(func);
  }
};

template<typename T>
typename detail::InstanceCollection<T> InstanceTracker<T>::s_collection;

} // namespace utils
