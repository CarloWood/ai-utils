// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class Sbll, a Simple Bidirectional Linked List, in namespace utils.
//
// Copyright (C) 2014, 2018 Carlo Wood.
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

#pragma once

#ifdef CWDEBUG
#include <libcwd/type_info.h>
#endif

/*
 * This file defines four base classes:
 *
 * class SbllNodeImpl,
 * class SbllListImpl,
 * class SbllNode<...> and
 * class SbllList<...>.
 *
 * They should be used to define a Bidirectional Linked List class as follows:
 *
 * #include "utils/Sbll.h"
 *
 * struct XxxxData { ... };     // Your data type.
 *
 * class XxxxNode: public SbllNode<SbllNodeImpl, XxxxNode>
 * {
 *   XxxxData data;	        // Stores your data
 *
 *   lteqgt_nt insert_cmp(XxxxNode const& Xxxx_data) const override
 *   {
 *     ...                     // return LESS, EQUAL or GREATER, when
 *                             //  Xxxx_data is less, equal or greater
 *                             //  then 'data' respectively.
 *   }
 *
 *   bool find_cmp(void const* key) const override
 *   {
 *     ...                     // return true or false, when 'key'
 *                             //  matches this node or not resp.
 *   }
 *
 *   void delinked(void) override
 *   {
 *     ...                     // Called when this node is delinked
 *   }
 *
 *   void done(void) override
 *   {
 *     ...                     // Called when this node is being removed
 *   }
 *
 *   void deceased(SbllNodeImpl* node, void* label) override
 *   {
 *     ...                     // Called when the crosslink parent node
 *                             //  is removed
 *   }
 * };
 *
 * and
 *
 * using XxxxList = SbllList<SbllListImpl, XxxxNode>;
 *
 * All the virtual member functions are optional and have a default action:
 *
 *   insert_cmp() : Defaults to inserting immedeately at the beginning of the list.
 *   find_cmp()   : Required if you use find() (the default coredumps :)).
 *   delinked()	  : Defaults to doing nothing.
 *   done()	  : Defaults to doing nothing.
 *   deceased()   : Defaults to removing the child (this node) too.
 *
 * Example:
 *
 * XxxxList foo("Xxxx");	// The parameter is used for debug output only.
 * XxxxNode* Xxxx_node;
 * XxxxNode* bar;
 * ...
 * bar = new XxxxNode(... Your initialisation parameters here ...);
 * foo.add(bar);		// Add 'bar' to list 'foo'.
 * Xxxx_node = foo.find(key);	// Find a node in list 'foo' with 'key'.
 * Xxxx_node->del();		// Remove/delete that node.
 *
 * Note that under some circumstances you might want to add attributes
 * to the list too. This can be done for example like this:
 *
 * class XxxxList : public SbllList<SbllListImpl, XxxxNode>
 * {
 *  public:
 *   // ... Your atributes/methods here
 *  private:
 *   init(...) { ... }
 *  public:
 *   XxxxList(void) : SbllList<SbllListImpl, XxxxNode>("XxxxList")
 *   { init(...); }
 *   XxxxList(char const* name) : SbllList<SbllListImpl, XxxxNode>(name)
 *   { init(...); }
 * };
 *
 * You ALWAYS need to define the *Data class first, and then the *List class.
 */

#pragma once

#include "debug.h"
#ifdef DEBUGLLISTS
#include <cstring>
#endif
//#include <libcw/support.h>

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct llists;
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {

// Forward declaration.
class SbllNodeImpl;
class SbllListImpl;

#ifdef DEBUGLLISTS
int const LLISTNAMESIZE = 16;
// I'd have liked to use an enum here, but gdb seems to have trouble
// with enum and large numbers.
typedef unsigned long magic_nt;
magic_nt const MAGICLIST   = 0xbcb3091a;
magic_nt const REMOVEDLIST = 0x87654321;
magic_nt const MAGICNODE   = 0xa530bf03;
magic_nt const REMOVEDNODE = 0x12345678;
#else
#  define whoami() "<unknown>"
#endif

/*
 * State of a node.
 * To save memory, we use the LSB of the pointer to the previous node
 * so we only have 1 bit. Therefore, 'SBLL_STATE_BEING_REMOVED' is
 * also used for a node that is being created.
 */
enum sbll_state_nt {
  SBLL_STATE_BEING_REMOVED = 0,
  SBLL_STATE_IN_USE = 1
};

/*****************************************************************************
 *
 * Class SbllBase
 *
 * Base class for the base classes, internal use only.
 *
 *****************************************************************************/
class SbllBase {
  friend class SbllNodeImpl;
  friend class SbllListImpl;
protected:
  SbllBase(void) { }
  ~SbllBase(void) { }
  unsigned long bitfield;
  SbllNodeImpl* next;		// Next node, or first node.
  SbllBase* prev(void) const
  {
    return (SbllBase*)(bitfield & ~1);
  }
  void set_prev(SbllBase* node)
  {
    if (bitfield & 1)
      bitfield = (unsigned long)node | 1;
    else
      bitfield = (unsigned long)node;
  }
  enum lteqgt_nt {
    LESS = -1,
    EQUAL = 0,
    GREATER = 1
  };
#ifdef DEBUGLLISTS
  magic_nt magicnumber;
#endif
};

/*****************************************************************************
 *
 * Class SbllListImpl
 *
 * Base class for your Bi-directional Linked Lists
 *
 *****************************************************************************/
class SbllListImpl : /*private  SEE BELOW*/ protected SbllBase {
private:
  void remove_all_nodes(void);
protected:
  using SbllBase::prev;	// Adjust access
  //using SbllBase::next;	// 3.5.0-20040530 cores on this - therefore inherite protected instead.
#ifdef DEBUGLLISTS
  using SbllBase::magicnumber;
#endif
  SbllListImpl(void)
  {
    Dout(dc::llists, "this = " << this << "; SbllListImpl()" );
    next = 0;				// Empty list
    set_prev((SbllBase*)-1);	// Mark this as being a list
#ifdef DEBUGLLISTS
    strncpy((char*)name, "<unknown sbll>", sizeof(name));
    ((char*)name)[sizeof(name) - 1] = '\0';
    magicnumber = MAGICLIST;
#endif
  }
#ifdef DEBUGLLISTS
  SbllListImpl(char const* listname)
#else
  SbllListImpl(char const* UNUSED_ARG(listname))
#endif
  {
#ifdef DEBUGLLISTS
    Dout(dc::llists, "this = " << this << "; SbllListImpl(\"" << listname << "\")" );
#endif
    next = 0;				// Empty list
    set_prev((SbllBase*)-1);	        // Mark this as being a list
#ifdef DEBUGLLISTS
    strncpy((char*)name, listname, sizeof(name));
    ((char*)name)[sizeof(name) - 1] = '\0';
    magicnumber = MAGICLIST;
#endif
  }
  ~SbllListImpl(void);			// Don't use 'virtual' for optimisation.
  					// This demands however that nobody ever
  					// fucks up with using a delete on this
  					// base class.
  void add(SbllNodeImpl* new_node, SbllNodeImpl* start_node);
  void insert_before(SbllNodeImpl* new_node, SbllNodeImpl* target_node);
  SbllNodeImpl* find(SbllNodeImpl* start_node, void const* key) const;
  SbllNodeImpl* find(void const* key) const
  {
    return find(next, key);
  }
public:
#ifdef DEBUGLLISTS
  char name[LLISTNAMESIZE];
  void check_consistency(void);
  void showlist(void);
#endif
};

/*****************************************************************************
 *
 * Class SbllNodeImpl
 *
 * Base class for your Bi-directional Linked Lists
 *
 *****************************************************************************/
class SbllNodeImpl : protected SbllBase {
  friend class SbllListImpl;
  //friend class cbll_node_ct;
private:
  sbll_state_nt get_state(void)
  {
    return (sbll_state_nt)(bitfield & 1);
  }
  void set_state(sbll_state_nt state)
  {
    if (state == SBLL_STATE_IN_USE)
      bitfield |= 1;
    else
      bitfield &= ~1;
  }
public:
  SbllNodeImpl(void)
  {
    Dout(dc::llists, "this = " << this << "; SbllNodeImpl()" );
    set_state(SBLL_STATE_BEING_REMOVED); /* Being created actually */
#ifdef DEBUGLLISTS
    set_prev(0);
    magicnumber = MAGICNODE;
#endif
  }
#ifdef DEBUGLLISTS
  virtual char const* whoami(void) const	// Return my actual type
  {
    return "SbllNodeImpl";
  }
#if CWDEBUG_ALLOC
  void print_whoami(void* that) const
#else
  void print_whoami(void* UNUSED_ARG(that)) const
#endif
  {
    Dout(dc::continued, " (" << whoami() );
#if CWDEBUG_ALLOC
    libcwd::alloc_ct const* alloc = libcwd::find_alloc(that);
    if (!alloc)
      Dout(dc::continued, " [ Not an allocated block! ]" );
    else if (alloc->start() != that)
      Dout(dc::continued, " at " << alloc->start());
#endif
    Dout(dc::continued, ')' );
  }
#endif
protected:
#ifndef DEBUGLLISTS
  virtual ~SbllNodeImpl(void) { }	// Destructor; protected: You _must_ use 'new' to make new objects.
  					// Every object derived from SbllNodeImpl destructs itself through
  					// SbllNodeImpl.
#else
  virtual ~SbllNodeImpl(void)
  {
    Dout(dc::llists|continued_cf, "this = " << this << "; ~SbllNodeImpl()" );
    print_whoami(this);
    Dout(dc::finish, "" );
    if (magicnumber != MAGICNODE)
      DoutFatal(dc::core, "SbllNodeImpl is already destructed!");
    if (get_state() == SBLL_STATE_IN_USE)
      DoutFatal(dc::core, "Destructing a node that is still in a list; You must use 'new' for nodes, and ->del() to delete them!");
    magicnumber = REMOVEDNODE;
  }
#endif
  using SbllBase::prev;	// Adjust access
  //using SbllBase::next;	// 3.5.0-20040530 cores on this - therefore inherite protected instead.
#ifdef DEBUGLLISTS
  using SbllBase::magicnumber;
#endif
  SbllNodeImpl* delink(void);
  SbllNodeImpl* del(void);
  virtual lteqgt_nt internal_insert_cmp(SbllBase const& UNUSED_ARG(listdata)) const = 0;
  virtual bool find_cmp(void const* UNUSED_ARG(key)) const = 0;
  virtual void delinked(void) = 0;
  virtual void done(void) = 0;
  virtual void deceased(SbllNodeImpl* UNUSED_ARG(node), void* UNUSED_ARG(label)) = 0;
  void insert_after(SbllNodeImpl* new_node);
  bool we_are_the_first(void) const
  {
    return (prev()->bitfield == (unsigned long)-1);
  }
};

/*****************************************************************************
 *
 * template SbllList
 *
 * Base class for your Bi-directional Linked Lists
 *
 *****************************************************************************/
template<class LIST_IMPL, class DATA_CT>
class SbllList: public LIST_IMPL {
public:
#ifdef DEBUGLLISTS
  SbllList(char const* listname) : LIST_IMPL(listname) { }
  SbllList(void) : LIST_IMPL("<unknown>") { }
#else
  SbllList(char const* DEBUG_ONLY(listname))
  {
    Dout(dc::llists, "this = " << this << "; " << libcwd::type_info_of(*this).demangled_name() << "(\"" << listname << "\")" );
  }
  SbllList(void) { }
#endif
  ~SbllList(void) { }   // Still not using virtual...
  inline DATA_CT* start_node(void)
  {
    return (DATA_CT*)LIST_IMPL::next;
  }
  inline DATA_CT const* const_start_node(void) const
  {
    return (DATA_CT const*)LIST_IMPL::next;
  }
  inline void add(DATA_CT* new_node, DATA_CT* start_node2)
  {
    LIST_IMPL::add(new_node, start_node2);
  }
  inline void add(DATA_CT* new_node)
  {
    LIST_IMPL::add((SbllNodeImpl*)new_node, (SbllNodeImpl*)LIST_IMPL::next);
  }
  inline void insert_before(DATA_CT* new_node, DATA_CT* target_node)
  {
    LIST_IMPL::insert_before((SbllNodeImpl*)new_node, (SbllNodeImpl*)target_node);
  }
  inline DATA_CT* find(DATA_CT* start_node2, void const* key)
  {
    return (DATA_CT*)LIST_IMPL::find(start_node2, key);
  }
  inline DATA_CT const* const_find(DATA_CT const* start_node2, void const* key) const
  {
    return (DATA_CT const*)LIST_IMPL::find((DATA_CT*)start_node2, key);
  }
  inline DATA_CT* find(void const* key)
  {
    return (DATA_CT*)LIST_IMPL::find(key);
  }
  inline DATA_CT const* const_find(void const* key) const
  {
    return (DATA_CT const*)LIST_IMPL::find(key);
  }
};

/*****************************************************************************
 *
 * template SbllNode
 *
 * Base class for your Bi-directional Linked Lists
 *
 *****************************************************************************/
template<class NODE_IMPL, class DATA_CT>
class SbllNode : public NODE_IMPL {
public:
  SbllNode(void) { }
  DATA_CT* next_node(void)
  {
    return (DATA_CT*)NODE_IMPL::next;
  }
  DATA_CT const* const_next_node(void) const
  {
    return (DATA_CT const*)NODE_IMPL::next;
  }
  DATA_CT* prev_node(void)
  {
    if (NODE_IMPL::we_are_the_first())		// Isn't it the list ?
      return 0;
    return (DATA_CT*)NODE_IMPL::prev();
  }
  DATA_CT const* const_prev_node(void) const
  {
    if (NODE_IMPL::we_are_the_first())		// Isn't it the list ?
      return 0;
    return (DATA_CT const*)NODE_IMPL::prev();
  }
  DATA_CT* delink(void)
  {
    return (DATA_CT*)NODE_IMPL::delink();
  }
  DATA_CT* del(void)
  {
    return (DATA_CT*)NODE_IMPL::del();
  }
  void insert_after(DATA_CT* new_node)
  {
    NODE_IMPL::insert_after((NODE_IMPL*)new_node);	// The cast is needed if DATA_CT privately inherites NODE_IMPL
  }
#ifdef DEBUGLLISTS
  virtual char const* whoami(void) const = 0;	  // Return my actual type
#endif

protected:
  virtual bool find_cmp(void const* UNUSED_ARG(key)) const
  {
    DoutFatal(dc::core, "If you want to use 'find()' you must define a find_cmp()");
    return false;	// Never reached.
  }
  virtual void delinked(void)
  {
    Dout(dc::llists, "this = " << this << "; Default delinked() (nothing to do)");
  }
  virtual void done(void)
  {
    Dout(dc::llists, "this = " << this << "; Default done() (nothing to do)");
  }
  virtual void deceased(SbllNodeImpl* DEBUG_ONLY(node), void* DEBUG_ONLY(label))
  {
    Dout(dc::llists, "this = " << this << "; Default deceased(" << node << ", " << label << "), " << "deleting this too");
    del();
  }
protected:
  virtual typename NODE_IMPL::lteqgt_nt insert_cmp(DATA_CT const& UNUSED_ARG(listdata)) const
  {
    Dout(dc::llists, "Using default " << ::libcwd::type_info_of(*this).demangled_name() << "::insert_cmp()");
    return NODE_IMPL::GREATER;	// Insert immedeately
  }
  virtual ~SbllNode(void) { };		// Use `new' to allocate new objects that inherit SbllNodeImpl

private:
  virtual typename NODE_IMPL::lteqgt_nt internal_insert_cmp(SbllBase const& listdata) const
      { return insert_cmp((DATA_CT const&)listdata); }
};

#ifndef DEBUGLLISTS
#  define DEBUGLLISTS_LLISTS_METHODS(x)
#else
#  define DEBUGLLISTS_LLISTS_METHODS(x) \
  char const* whoami(void) const override /* Return my actual type */ \
      { return #x; }
#endif

#define LLISTS_METHODS(class_type) \
  protected: \
    DEBUGLLISTS_LLISTS_METHODS(class_type) \
    ~class_type(void) { }
      // Always use 'new' to allocate a new object derived from `SbllNodeImpl'.
      // Never delete it, call method `del()' instead.

#define LLISTS_DTOR(class_type) \
  protected: \
    DEBUGLLISTS_LLISTS_METHODS(class_type) \
    ~class_type(void)
      // Always use 'new' to allocate a new object derived from `SbllNodeImpl'.
      // Never delete it, call method `del()' instead.

} // namespace utils
