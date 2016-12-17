/**
 * \file translate.h
 * \brief Declaraction of namespace translate.
 *
 * Copyright (C) 2014, 2016  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
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

#ifndef UTILS_TRANSLATE_H
#define UTILS_TRANSLATE_H

#ifndef USE_PCH
#include <map>
#include <string>
#endif

namespace translate {

typedef std::map<std::string, std::string> format_map_t;
std::string getString(std::string const& xmlDesc, format_map_t const& format_map);

} // namespace translate

#endif // UTILS_TRANSLATE_H
