/**
 * \file GlobalObjectManager.cpp
 * \brief Implementation of class GlobalObjectManager.
 *
 * Copyright (C) 2014 Aleric Inglewood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \class GlobalObjectManager
 * \brief Global objects manager singleton class.
 *
 * This singleton is used by Global<> to keep track of the number
 * of global instances and their destruction.
 */

#ifndef USE_PCH
#include "sys.h"
#endif

#include "GlobalObjectManager.h"

#ifndef USE_PCH
#include "debug.h"
#ifdef CWDEBUG
#include <libcwd/cwprint.h>
#endif
#endif

using namespace utils::_internal_;

#ifdef DEBUGGLOBAL
bool GlobalObjectManager::after_global_constructors = false;
#endif

/// @cond Doxygen_Suppress
void GlobalObjectManager::registerGlobalObject(GlobalObject* globalObject)
{
  globalObjects.push_back(globalObject);
}

void GlobalObjectManager::deleteGlobalObjects(void)
{
  bool done;
  do
  {
    GlobalObject* globalObject = globalObjects.back();
    globalObjects.pop_back();
    done = globalObjects.empty();
    if (!done)				// Don't call the destructor of GlobalObjectManager itself! (last one is self)
      globalObject->~GlobalObject();
  }
  while(!done);
}
/// @endcond

#ifdef DEBUGGLOBAL
void GlobalObjectManager::main_entered(void)
{
  Singleton<GlobalObjectManager>::instantiate();
  for (globalObjects_type::const_iterator i(Singleton<GlobalObjectManager>::instance().globalObjects.begin());
       i != Singleton<GlobalObjectManager>::instance().globalObjects.end();
       ++i)
  {
    if (!(*i)->instantiated_from_constructor())
    {
      DoutFatal( dc::core,
	  "Missing global/static initialization of `" << cwprint_using(*(*i), &GlobalObject::print_type_name) << "'.\n"
	  "          There should be one and only one code line reading:\n"
	  "          static " << cwprint_using(*(*i), &GlobalObject::print_type_name) << " dummy;" );
    }
    (*i)->set_initialized_and_after_global_constructors();
  }
  Singleton<GlobalObjectManager>::instance().after_global_constructors = true;
}
#endif

namespace {
  static SingletonInstance<GlobalObjectManager> GlobalObjectManager_instance __attribute__ ((unused));
}
