/**
 * \file aialert.cpp
 * \brief Implementation of AIAlert
 *
 * Copyright (C) 2013, 2016  Carlo Wood.
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
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   02/11/2013
 *   - Initial version, written by Aleric Inglewood @ SL
 *
 *   05/11/2013
 *   - Moved everything in namespace AIAlert, except AIArgs.
 *
 *   31/08/2014
 *   - Copied the code from SingularityViewer and relicensed it to have
 *     no license (allowed because I was the only author of this code).
 *
 *   2014/12/24
 *   - Added Affero GPL v3 license and Released publically on github.
 *
 *   2016/12/17
 *   - Transfered copyright to Carlo Wood.
 */

#include "AIAlert.h"

namespace AIAlert {

Error::Error(Prefix const& prefix, modal_nt type,
             Error const& alert) : mLines(alert.mLines), mModal(type)
{
  if (alert.mModal == modal) mModal = modal;
  if (prefix) mLines.push_front(Line(prefix));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc, AIArgs const& args) : mModal(type)
{
  if (prefix) mLines.push_back(Line(prefix));
  mLines.push_back(Line(xml_desc, args));
}

Error::Error(Prefix const& prefix, modal_nt type,
             Error const& alert,
             std::string const& xml_desc, AIArgs const& args) : mLines(alert.mLines), mModal(type)
{
  if (alert.mModal == modal) mModal = modal;
  if (prefix) mLines.push_back(Line(prefix, !mLines.empty()));
  mLines.push_back(Line(xml_desc, args));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc,
             Error const& alert) : mLines(alert.mLines), mModal(type)
{
  if (alert.mModal == modal) mModal = modal;
  if (!mLines.empty()) { mLines.front().set_newline(); }
  mLines.push_front(Line(xml_desc));
  if (prefix) mLines.push_front(Line(prefix));
}

Error::Error(Prefix const& prefix, modal_nt type,
             std::string const& xml_desc, AIArgs const& args,
             Error const& alert) : mLines(alert.mLines), mModal(type)
{
  if (alert.mModal == modal) mModal = modal;
  if (!mLines.empty()) { mLines.front().set_newline(); }
  mLines.push_front(Line(xml_desc, args));
  if (prefix) mLines.push_front(Line(prefix));
}

} // namespace AIAlert
