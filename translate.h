// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaraction of namespace translate.
 */

#pragma once

#include <map>
#include <string>

namespace translate {

using format_map_t = std::map<std::string, std::string>;
std::string getString(std::string const& xmlDesc, format_map_t const& format_map);

} // namespace translate
