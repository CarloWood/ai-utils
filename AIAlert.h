// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Declaration of AIArgs and AIAlert classes.
//
// Copyright (C) 2013, 2016  Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
//
// CHANGELOG
//   and additional copyright holders.
//
//   2013/11/02
//   - Initial version, written by Aleric Inglewood @ SL
//
//   2013/11/05
//   - Moved everything in namespace AIAlert, except AIArgs.
//
//   2014/08/31
//   - Copied the code from SingularityViewer and relicensed it to have
//     no license (allowed because I was the only author of this code).
//
//   2014/12/24
//   - Added Affero GPL v3 license and Released publically on github.
//
//   2016/12/17
//   - Transfered copyright to Carlo Wood.
//
//   2018/01/02
//   - Changed license to GPL-3.

#pragma once

#include "translate.h"

#include <deque>
#include <exception>
#include <string>
#include <map>
#include <boost/lexical_cast.hpp>

//===================================================================================================================================
// Facility to throw errors that can easily be converted to an informative pop-up floater for the user.

// Throw arbitrary class.
/// Throw arbitrary class \a Alert - not modal, no function prefix.
#define   THROW_ALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(),                                                     AIAlert::not_modal, __VA_ARGS__)
/// Throw arbitrary class \a Alert - modal, no function prefix.
#define  THROW_MALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(),                                                         AIAlert::modal, __VA_ARGS__)
#ifdef __GNUC__
/// Throw arbitrary class \a Alert - modal, no function prefix.
#define  THROW_FALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(__PRETTY_FUNCTION__, AIAlert::pretty_function_prefix), AIAlert::not_modal, __VA_ARGS__)
/// Throw arbitrary class \a Alert - modal, with function prefix.
#define THROW_FMALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(__PRETTY_FUNCTION__, AIAlert::pretty_function_prefix),     AIAlert::modal, __VA_ARGS__)
#else
/// @cond Doxygen_Suppress
#define  THROW_FALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(__FUNCTION__, AIAlert::pretty_function_prefix), AIAlert::not_modal, __VA_ARGS__)
#define THROW_FMALERT_CLASS(Alert, ...) throw Alert(AIAlert::Prefix(__FUNCTION__, AIAlert::pretty_function_prefix),     AIAlert::modal, __VA_ARGS__)
/// @endcond
#endif

// Shortcut to throw AIAlert::Error.
/// Throw alert message (AIAlert::Error) - not modal, no function prefix.
#define   THROW_ALERT(...)   THROW_ALERT_CLASS(AIAlert::Error, __VA_ARGS__)
/// Throw alert message (AIAlert::Error) - modal, no function prefix.
#define  THROW_MALERT(...)  THROW_MALERT_CLASS(AIAlert::Error, __VA_ARGS__)
/// Throw alert message (AIAlert::Error) - not modal, with function prefix.
#define  THROW_FALERT(...)  THROW_FALERT_CLASS(AIAlert::Error, __VA_ARGS__)
/// Throw alert message (AIAlert::Error) - modal, with function prefix.
#define THROW_FMALERT(...) THROW_FMALERT_CLASS(AIAlert::Error, __VA_ARGS__)

// Shortcut to throw AIAlert::ErrorCode.
/// Throw alert message with error code (AIAlert::ErrorCode) - not modal, no function prefix.
#define   THROW_ALERTC(...)   THROW_ALERT_CLASS(AIAlert::ErrorCode, __VA_ARGS__)
/// Throw alert message with error code (AIAlert::ErrorCode) - modal, no function prefix.
#define  THROW_MALERTC(...)  THROW_MALERT_CLASS(AIAlert::ErrorCode, __VA_ARGS__)
/// Throw alert message with error code (AIAlert::ErrorCode) - not modal, with function prefix.
#define  THROW_FALERTC(...)  THROW_FALERT_CLASS(AIAlert::ErrorCode, __VA_ARGS__)
/// Throw alert message with error code (AIAlert::ErrorCode) - modal, with function prefix.
#define THROW_FMALERTC(...) THROW_FMALERT_CLASS(AIAlert::ErrorCode, __VA_ARGS__)

// Shortcut to throw AIAlert::ErrorCode with errno as code.
/// Throw alert message (AIAlert::ErrorCode) with errno as error code - not modal, no function prefix.
#define   THROW_ALERTE(...) do { int errn = errno;   THROW_ALERT_CLASS(AIAlert::ErrorCode, errn, __VA_ARGS__); } while(0)
/// Throw alert message (AIAlert::ErrorCode) with errno as error code - modal, no function prefix.
#define  THROW_MALERTE(...) do { int errn = errno;  THROW_MALERT_CLASS(AIAlert::ErrorCode, errn, __VA_ARGS__); } while(0)
/// Throw alert message (AIAlert::ErrorCode) with errno as error code - not modal, with function prefix.
#define  THROW_FALERTE(...) do { int errn = errno;  THROW_FALERT_CLASS(AIAlert::ErrorCode, errn, __VA_ARGS__); } while(0)
/// Throw alert message (AIAlert::ErrorCode) with errno as error code - modal, with function prefix.
#define THROW_FMALERTE(...) do { int errn = errno; THROW_FMALERT_CLASS(AIAlert::ErrorCode, errn, __VA_ARGS__); } while(0)

// Examples

#ifdef EXAMPLE_CODE

  //----------------------------------------------------------
  // To show the alert box:

  catch (AIAlert::Error const& error)
  {
     AIAlert::add(error);        // Optionally pass pretty_function_prefix as second parameter to *suppress* that output.
  }

  // or, for example

  catch (AIAlert::ErrorCode const& error)
  {
     if (error.getCode() != EEXIST)
     {
       AIAlert::add(error, AIAlert::pretty_function_prefix);
     }
  }
  //----------------------------------------------------------
  // To throw alerts:

  THROW_ALERT("ExampleKey");                                                               // A) Lookup "ExampleKey" in strings.xml and show translation.
  THROW_ALERT("ExampleKey", AIArgs("[FIRST]", first)("[SECOND]", second)(...etc...));      // B) Same as A, but replace [FIRST] with first, [SECOND] with second, etc.
  THROW_ALERT("ExampleKey", error);                                                        // C) As A, but followed by a colon and a newline, and then the text of 'error'.
  THROW_ALERT(error, "ExampleKey");                                                        // D) The text of 'error', followed by a colon and a newline and then as A.
  THROW_ALERT("ExampleKey", AIArgs("[FIRST]", first)("[SECOND]", second), error);          // E) As B, but followed by a colon and a newline, and then the text of 'error'.
  THROW_ALERT(error, "ExampleKey", AIArgs("[FIRST]", first)("[SECOND]", second));          // F) The text of 'error', followed by a colon and a newline and then as B.
  // where 'error' is a caught Error object (as above) in a rethrow.
  // Prepend ALERT with F and/or M to prepend the text with the current function name and/or make the alert box Modal.
  // For example,
  THROW_FMALERT("ExampleKey", AIArgs("[FIRST]", first));    // Throw a Modal alert box that is prefixed with the current Function name.
  // Append E after ALERT to throw an ErrorCode class that contains the current errno.
  // For example,
  THROW_FALERTE("ExampleKey", AIArgs("[FIRST]", first));    // Throw an alert box that is prefixed with the current Function name and pass errno to the catcher.

#endif // EXAMPLE_CODE

//
//===================================================================================================================================

/**
 * \class AIArgs
 * \brief Arguments for AIAlert::Error.
 *
 * A wrapper around a `std::map` (translate::format_map_t) to allow constructing a dictionary on one line by doing:
 *
 * <code>AIArgs("[ARG1]", arg1)("[ARG2]", arg2)("[ARG3]", arg3)...</code>
 */

class AIArgs
{
  private:
    translate::format_map_t mArgs;    ///< The underlying replacement map.

  public:
    /// Construct an empty map.
    AIArgs() { }
    /// Construct a map with a single replacement.
    template<typename T> AIArgs(char const* key, T const& replacement) { mArgs[key] = boost::lexical_cast<std::string>(replacement); }
    /// Add another replacement.
    template<typename T> AIArgs& operator()(char const* key, T const& replacement) { mArgs[key] = boost::lexical_cast<std::string>(replacement); return *this; }
    /// The destructor may not throw.
    ~AIArgs() throw() { }

    /// Accessor, returns the underlaying map.
    translate::format_map_t const& operator*() const { return mArgs; }
};

// No need to call boost::lexical_cast when it already is a std::string or a char const*.
/// Map \a replacement to \a key. Specialization for `std::string`.
template<> inline AIArgs::AIArgs(char const* key, std::string const& replacement) { mArgs[key] = replacement; }
/// Map \a replacement to \a key. Specialization for `char const*`.
template<> inline AIArgs::AIArgs(char const* key, char const* const& replacement) { mArgs[key] = replacement; }
/// Map \a replacement to \a key. Specialization for `std::string`.
template<> inline AIArgs& AIArgs::operator()(char const* key, std::string const& replacement) { mArgs[key] = replacement; return *this; }
/// Map \a replacement to \a key. Specialization for `char const*`.
template<> inline AIArgs& AIArgs::operator()(char const* key, char const* const& replacement) { mArgs[key] = replacement; return *this; }

namespace AIAlert {

/// Whether or not an alert should be modal.
enum modal_nt
{
  not_modal,
  modal
};

/// Mask values that detemine prefix format.
enum alert_line_type_nt
{
  normal = 0,			///< Empty mask, used for normal lines.
  empty_prefix = 1,		///< Mask bit for an empty prefix.
  pretty_function_prefix = 2	///< Mask bit for a function name prefix.
  // These must exist of single bits (a mask).
};

/**
 * \class Prefix
 * \brief The prefix of an alert message.
 *
 * An Prefix currently comes only in two flavors:
 *
 * empty_prefix           : An empty prefix.
 * pretty_function_prefix : A function name prefix, this is the function from which the alert was thrown.
 */

class Prefix
{
  public:
    /// Construct an empty Prefix.
    Prefix() : mType(empty_prefix) { }
    /// Construct a prefix \a str of type \a type.
    Prefix(char const* str, alert_line_type_nt type) : mStr(str), mType(type) { }
    /// Construct a prefix \a str of type \a type.
    Prefix(std::string str, alert_line_type_nt type) : mStr(str), mType(type) { }

    /// Return true if the prefix is not empty.
    operator bool() const { return mType != empty_prefix; }
    /// Accessor for the type of the prefix.
    alert_line_type_nt type() const { return mType; }
    /// Accessor for the prefix string.
    std::string const& str() const { return mStr; }

  private:
    std::string mStr;                ///< Literal text. For example a C++ function name.
    alert_line_type_nt mType;        ///< The type of this prefix.
};

/**
 * \class Line
 * \brief A single line of an alert message.
 *
 * This class represents one line with its replacements.
 * The string mXmlDesc shall be looked up in strings.xml.
 * This is not done as part of this class so that the
 * actual translation code can be defined elsewhere and
 * used more generally.
 */

class Line
{
  private:
    bool mNewline;                   ///< Prepend this line with a newline if set.
    std::string mXmlDesc;            ///< The keyword to look up in string.xml.
    AIArgs mArgs;                    ///< Replacement map.
    alert_line_type_nt mType;        ///< The type of this line: normal for normal lines, other for prefixes.

  public:
    /// Construct a line with no replacements.
    Line(std::string const& xml_desc, bool newline = false) : mNewline(newline), mXmlDesc(xml_desc), mType(normal) { }
    /// Construct a line with replacements \a args.
    Line(std::string const& xml_desc, AIArgs const& args, bool newline = false) : mNewline(newline), mXmlDesc(xml_desc), mArgs(args), mType(normal) { }
    /// Construct a prefix line.
    Line(Prefix const& prefix, bool newline = false) : mNewline(newline), mXmlDesc("[PREFIX]"), mArgs("[PREFIX]", prefix.str()), mType(prefix.type()) { }
    /// The destructor may not throw.
    ~Line() throw() { }

    /// Prepend a newline before this line.
    void set_newline() { mNewline = true; }

    // These are to be used like: translate::getString(line.getXmlDesc(), line.args()) and prepend with a \n if prepend_newline() returns true.
    /// Return the xml key.
    std::string getXmlDesc() const { return mXmlDesc; }
    /// Accessor for the replacement map.
    translate::format_map_t const& args() const { return *mArgs; }
    /// Returns true a new line must be prepended before this line.
    bool prepend_newline() const { return mNewline; }

    // Accessors.
    /// Return true if this Line must be suppressed.
    bool suppressed(unsigned int suppress_mask) const { return (suppress_mask & mType) != 0; }
    /// Return true if this is a prefix Line.
    bool is_prefix() const { return mType != normal; }
};

/**
 * \class Error
 * \brief Exception class used to throw an error with an informative user message.
 *
 * This class is used to throw an error that will cause
 * an alert box to pop up for the user.
 *
 * An alert box only has text and an OK button.
 * The alert box does not give feed back to the program; it is purely informational.
 *
 * The class represents multiple lines, each Line is to be translated and catenated,
 * separated by newlines, and then written to an alert box. This is not done as part
 * of this class so that the code that shows messages that have to be translated
 * can be defined elsewhere and used more general.
 */

class Error : public std::exception
{
  public:
    using lines_type = std::deque<Line>; ///< The type of mLines.

    /// The destructor may not throw.
    ~Error() throw() { }

    // Accessors.
    /// Accessor for the lines deque.
    lines_type const& lines() const { return mLines; }
    /// Return true if the alert should be modal.
    bool is_modal() const { return mModal == modal; }

    /// Existing \a alert, just add a prefix and turn alert into modal if appropriate.
    Error(Prefix const& prefix, modal_nt type, Error const& alert);

    /// A string with zero or more replacements (with args).
    Error(Prefix const& prefix, modal_nt type,
          std::string const& xml_desc, AIArgs const& args = AIArgs());

    /// A string (with args), prepending the message with the text of another alert.
    Error(Prefix const& prefix, modal_nt type,
          Error const& alert,
          std::string const& xml_desc, AIArgs const& args = AIArgs());

    /// A string (no args), appending the message with the text of another alert.
    Error(Prefix const& prefix, modal_nt type,
          std::string const& xml_desc,
          Error const& alert);
    /// A string (with args), appending the message with the text of another alert.
    Error(Prefix const& prefix, modal_nt type,
          std::string const& xml_desc, AIArgs const& args,
          Error const& alert);

  private:
    lines_type mLines;		///< The lines (or prefixes) of text to be displayed, each consisting of a keyword (to be looked up in strings.xml) and a replacement map.
    modal_nt mModal;		///< If true, make the alert box a modal floater.
};

/**
 * \class ErrorCode
 * \brief Exception class used to throw an error with an informative user message.
 *
 * Same as Error but allows to pass an additional error code.
 */

class ErrorCode : public Error
{
  private:
    int mCode;		///< The underlaying error code.

  public:
    /// The destructor may not throw.
    ~ErrorCode() throw() { }

    /// Accessor for the error code.
    int getCode() const { return mCode; }

    /// Construct just an Error with a code (no args).
    ErrorCode(Prefix const& prefix, modal_nt type, int code,
              Error const& alert) :
      Error(prefix, type, alert), mCode(code) { }

    /// Construct a string with zero or more replacements (with args).
    ErrorCode(Prefix const& prefix, modal_nt type, int code,
              std::string const& xml_desc, AIArgs const& args = AIArgs()) :
      Error(prefix, type, xml_desc, args), mCode(code) { }

    /// Construct a string (with args), prepending the message with the text of another alert.
    ErrorCode(Prefix const& prefix, modal_nt type, int code,
              Error const& alert,
              std::string const& xml_desc, AIArgs const& args = AIArgs()) :
      Error(prefix, type, alert, xml_desc, args), mCode(code) { }

    /// Construct a string (no args), appending the message with the text of another alert (no args).
    ErrorCode(Prefix const& prefix, modal_nt type, int code,
              std::string const& xml_desc,
              Error const& alert) :
      Error(prefix, type, xml_desc, alert), mCode(code) { }
    /// Construct an Error with a code, appending the message with the text of another alert (with args).
    ErrorCode(Prefix const& prefix, modal_nt type, int code,
              std::string const& xml_desc, AIArgs const& args,
              Error const& alert) :
      Error(prefix, type, xml_desc, args, alert), mCode(code) { }
};

} // namespace AIAlert
