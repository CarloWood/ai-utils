/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of AIAlert.
 *
 * @Copyright (C) 2013, 2016  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
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
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   2013/11/02
 *   - Initial version, written by Aleric Inglewood @ SL
 *
 *   2013/11/05
 *   - Moved everything in namespace AIAlert, except AIArgs.
 *
 *   2014/08/31
 *   - Copied the code from SingularityViewer and relicensed it to have
 *     no license (allowed because I was the only author of this code).
 *
 *   2014/12/24
 *   - Added Affero GPL v3 license and Released publically on github.
 *
 *   2016/12/17
 *   - Transfered copyright to Carlo Wood.
 *
 *   2018/01/02
 *   - Changed license to GPL-3.
 */

#include "AIAlert.h"

namespace AIAlert {

Error::Error(Prefix const& prefix, modal_nt type,
             Error const& alert) : mLines(alert.mLines), mModal(type), mErrorCode(false)
{
  if (alert.mModal == modal) mModal = modal;
  if (prefix) mLines.push_front(Line(prefix));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc, AIArgs const& args) : mModal(type), mErrorCode(false)
{
  if (prefix) mLines.push_back(Line(prefix));
  mLines.push_back(Line(xml_desc, args));
}

Error::Error(Prefix const& prefix, modal_nt type,
             Error const& alert,
             std::string const& xml_desc, AIArgs const& args) : mLines(alert.mLines), mModal(type), mErrorCode(false)
{
  if (alert.mModal == modal) mModal = modal;
  if (prefix) mLines.push_back(Line(prefix, !mLines.empty()));
  mLines.push_back(Line(xml_desc, args));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc,
             Error const& alert) : mLines(alert.mLines), mModal(type), mErrorCode(false)
{
  if (alert.mModal == modal) mModal = modal;
  if (!mLines.empty()) { mLines.front().set_newline(); }
  mLines.push_front(Line(xml_desc));
  if (prefix) mLines.push_front(Line(prefix));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc, AIArgs const& args,
             Error const& alert) : mLines(alert.mLines), mModal(type), mErrorCode(false)
{
  if (alert.mModal == modal) mModal = modal;
  if (!mLines.empty()) { mLines.front().set_newline(); }
  mLines.push_front(Line(xml_desc, args));
  if (prefix) mLines.push_front(Line(prefix));
}

void ErrorCode::finish_init()
{
  mErrorCode = true;
  if (is_prefix())
    mLines.push_front(Prefix(mCode.message() + ": ", error_code));
  else
    mLines.push_back(": " + mCode.message());
}

} // namespace AIAlert

