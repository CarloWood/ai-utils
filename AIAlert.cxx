// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of AIAlert.
 *
 * This file is part of ai-utils.
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
 *
 *   2022/04/16
 *   - Updated GPG key with newer version (from 2018).
 *
 *   2026/07/21
 *   - Changed license to MIT.
 */

#include "sys.h"
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
  bool prepend_newline = false;
  if (prefix) mLines.push_back(Line(prefix, !mLines.empty()));
  else prepend_newline = true;
  mLines.push_back(Line(xml_desc, args, prepend_newline));
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
