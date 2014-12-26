/**
 * \file debug_ostream_operators.cpp
 * \brief This file contains the definition of debug serializers.
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
 */

#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#ifdef CWDEBUG

#include "translate.h"
#include "utils/AIAlert.h"

#include <iostream>

//! For debugging purposes. Write a AIAlert::Error to \a os.
std::ostream& operator<<(std::ostream& os, AIAlert::Error const& error)
{
  os << "AIAlert: ";
  int lines = error.lines().size();
  int count = 0;
  for (AIAlert::Error::lines_type::const_iterator line = error.lines().begin(); line != error.lines().end(); ++line)
  {
    if (++count < lines && lines > 1) os << "\n    ";
    os << translate::getString(line->getXmlDesc(), line->args());
  }
  return os;
}

#endif // CWDEBUG
