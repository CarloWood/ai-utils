/**
 * \file debug_ostream_operators.h
 * \brief This file contains the declaration of debug serializers.
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

#ifndef UTILS_DEBUG_OSTREAM_OPERATORS_H
#define UTILS_DEBUG_OSTREAM_OPERATORS_H

#ifdef CWDEBUG

#include <iosfwd>                       // std::ostream&

namespace AIAlert {
  class Error;
} // namespace AIAlert

extern std::ostream& operator<<(std::ostream& os, AIAlert::Error const& error);

#endif // CWDEBUG
#endif // UTILS_DEBUG_OSTREAM_OPERATORS_H
