/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of sbll.
 *
 * @Copyright (C) 2014, 2018  Carlo Wood.
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
 */

#include "sys.h"
#include "debug.h"
#include "sbll.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
channel_ct llists("LLISTS");
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils {

void SbllNodeImpl::insert_after(SbllNodeImpl* new_node)
{
#ifdef DEBUGLLISTS
  if (magicnumber != MAGICNODE)
    DoutFatal(dc::core, "SbllNodeImpl::insert_after(): Inserting after an invalid node!");
#endif
#ifdef CWDEBUG
  if (!new_node)
    DoutFatal(dc::core, "SbllNodeImpl::insert_after(): You must give a node to insert");
#endif
#ifdef DEBUGLLISTS
  if (new_node->magicnumber != MAGICNODE)
    DoutFatal(dc::core, "SbllNodeImpl::insert_after(): Trying to insert an invalid node");
#endif
#ifdef CWDEBUG
  if (new_node->get_state() == SBLL_STATE_IN_USE)
    DoutFatal(dc::core, "Inserting a node twice");
#endif
  new_node->set_state(SBLL_STATE_IN_USE);
  new_node->set_prev(this);
  if ((new_node->next = next))
    next->set_prev(new_node);
  next = new_node;
}

/*
 * add
 *
 * Adds data to the list, starts looking for the right place
 * (using internal_insert_cmp) at node start. If 'start' is not known,
 * use the inline function add(SbllNodeImpl const& data_node),
 * which starts looking at the beginning of the list.
 */
void SbllListImpl::add(SbllNodeImpl* new_node, SbllNodeImpl* start_node)
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "this = " << this << " (\"" << name << "\"); SbllListImpl::add(" << new_node << ", " << start_node << ")");
  if (magicnumber != MAGICLIST)
    DoutFatal(dc::core, "Not a list");
  if (start_node)
  {
    if (start_node->magicnumber != MAGICNODE)
      DoutFatal(dc::core, "start_node has wrong magicnumber!");
    SbllNodeImpl* node = start_node;
    while (node != next)
    {
      if (node->prev()->magicnumber == MAGICLIST)
        DoutFatal(dc::core, "start_node is not a node of " << name << " but of " << ((SbllListImpl*)node->prev())->name);
      node = (SbllNodeImpl*)node->prev();
      if (node->magicnumber != MAGICNODE)
        DoutFatal(dc::core, "start_node in corrupted chain");
    }
  }
  else if (next)
    DoutFatal(dc::core, "start_node NULL, but list (\"" << name << "\") not empty!");
  if (new_node->magicnumber == REMOVEDNODE)
    DoutFatal(dc::core, "Trying to add a node that was previously removed");
  if (new_node->magicnumber != MAGICNODE)
    DoutFatal(dc::core, "Trying to add a corrupted node");
#endif
#ifdef CWDEBUG
  if (new_node->get_state() == SBLL_STATE_IN_USE)
    DoutFatal(dc::core, "Adding a node twice");
#endif
  new_node->set_state(SBLL_STATE_IN_USE);

  SbllBase* p1;
  SbllNodeImpl* p2;

  if (!(p2 = start_node)) // List empty ?
  {
    /* Insert first node */
    next = new_node;
    new_node->set_prev(this);
    new_node->next = 0;
    return;
  }

  /* Look for right place to insert: between p1 and p2 */
  lteqgt_nt d;
  if ((d = new_node->internal_insert_cmp(*p2)) == EQUAL ||
      (p2 == next && d == GREATER))
  {
     /* Immedeately insert before the starting point */
     p1 = p2->prev();
  }
  else if (d == GREATER) /* Started behond the right node? */
  {
    // Look backwards through list till we find a smaller one,
    // or hit the end of the list.
    do
    {
      p2 = (SbllNodeImpl*)p2->prev();	// cast: p2 != next
    }
    while ((d = new_node->internal_insert_cmp(*p2)) == GREATER && p2 != next);
    if (d == GREATER)
      p1 = p2->prev();
    else
    {
      p1 = p2;
      p2 = p1->next;
    }
  }
  else /* d == LESS; insert after p2 */
  {
    // Look forwards till we find a larger one,
    // or hit the end of the list.
    do
    {
      p1 = p2;
      p2 = p1->next;
    }
    while (p2 && new_node->internal_insert_cmp(*p2) == LESS);
  }

#ifdef DEBUGLLISTS
    if (p1 && p1->magicnumber != MAGICNODE && p1->magicnumber != MAGICLIST)
      DoutFatal(dc::core, "Trying to insert after an invalid node!");
    if (p2 && p2->magicnumber != MAGICNODE)
      DoutFatal(dc::core, "Trying to insert before an invalid node!");
#endif

    /* Insert before p2: */
    p1->next = new_node;
    if (p2)
      p2->set_prev(new_node);
    new_node->next = p2;
    new_node->set_prev(p1);
}

/*
 * insert_before
 *
 * Adds data to the list, inserting before `target_node'.
 * If 'target_node' is nullptr, then insert at the beginning of the list.
 * This code is mostly equal to the code of `add'.
 */
void SbllListImpl::insert_before(SbllNodeImpl* new_node, SbllNodeImpl* target_node)
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "this = " << this << " (\"" << name << "\"); SbllListImpl::insert_before(" << new_node << ", " << target_node << ")");
  if (magicnumber != MAGICLIST)
    DoutFatal(dc::core, "Not a list");
  if (target_node)
  {
    if (target_node->magicnumber != MAGICNODE)
      DoutFatal(dc::core, "target_node has wrong magicnumber!");
    SbllNodeImpl* node = target_node;
    while (node != next)
    {
      if (node->prev()->magicnumber == MAGICLIST)
        DoutFatal(dc::core, "target_node is not a node of " << name << " but of " << ((SbllListImpl*)node->prev())->name);
      node = (SbllNodeImpl*)node->prev();
      if (node->magicnumber != MAGICNODE)
        DoutFatal(dc::core, "target_node in corrupted chain");
    }
  }
  else if (next)
    DoutFatal(dc::core, "target_node NULL, but list (\"" << name << "\") not empty!");
  if (new_node->magicnumber == REMOVEDNODE)
    DoutFatal(dc::core, "Trying to add a node that was previously removed");
  if (new_node->magicnumber != MAGICNODE)
    DoutFatal(dc::core, "Trying to add a corrupted node");
#endif
#ifdef CWDEBUG
  if (new_node->get_state() == SBLL_STATE_IN_USE)
    DoutFatal(dc::core, "Adding a node twice");
#endif
  new_node->set_state(SBLL_STATE_IN_USE);

  SbllBase* p1;
  SbllNodeImpl* p2;

  if (!(p2 = target_node)) // List empty ?
  {
    /* Insert first node */
    next = new_node;
    new_node->set_prev(this);
    new_node->next = 0;
    return;
  }
  p1 = p2->prev();

#ifdef DEBUGLLISTS
    if (p1 && p1->magicnumber != MAGICNODE && p1->magicnumber != MAGICLIST)
      DoutFatal(dc::core, "Trying to insert after an invalid node!");
    if (p2 && p2->magicnumber != MAGICNODE)
      DoutFatal(dc::core, "Trying to insert before an invalid node!");
#endif

    /* Insert before p2: */
    p1->next = new_node;
    if (p2)
      p2->set_prev(new_node);
    new_node->next = p2;
    new_node->set_prev(p1);
}

/*
 * delink (This code is very similar to that of del() below)
 *
 * Delinks node from the list.
 * Returns a pointer to the first next node that is still in the chain,
 * or nullptr if there is no next node (used for running over all nodes
 * in a loop that can delink/remove them).
 */
SbllNodeImpl* SbllNodeImpl::delink()
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "SbllNodeImpl::delink() " << this);
  if (magicnumber == REMOVEDNODE)
    DoutFatal(dc::core, "Trying to delink a node twice");
  if (magicnumber != MAGICNODE)
    DoutFatal(dc::core, "Trying to delink a non-existant node!");
  if (!prev() || prev()->next != this)
    DoutFatal(dc::core, "Trying to delink a node that is not part of a list");
#endif

  /* Is this node already in the process of being removed ? */
  if (get_state() == SBLL_STATE_BEING_REMOVED)
  {
    for(SbllNodeImpl* p = next; p; p = p->next)
      if (p->get_state() == SBLL_STATE_IN_USE)
	return p;		// Return first node that is not being removed
    return nullptr;		// No nodes left
  }
  set_state(SBLL_STATE_BEING_REMOVED);

  delinked();			// Call user function
  				// This is now allowed to call del()
  				// for any node, even nodes that are
  				// in the process of being delinked/removed.

  /* Find the next non-removed node if any */
  SbllNodeImpl* nextnode;
  for(nextnode = next; nextnode; nextnode = nextnode->next)
    if (nextnode->get_state() == SBLL_STATE_IN_USE)
      break;

  /* Delink this node from the chain: */
  if (prev()) // Didn't we just remove the list itself ?
  {
    if ((prev()->next = next))
      next->set_prev(prev());
  }
  else if (next)
  {
#ifdef CWDEBUG
    if (next->get_state() != SBLL_STATE_BEING_REMOVED)
      DoutFatal(dc::core, "'next' node not removed, but list is gone!?");
#endif
    next->set_prev(0); // Tell the next node too that the list is gone.
  }

  Dout(dc::llists, "this = " << this << "; delinking " << this << " (this)");

  return (SbllNodeImpl*)nextnode;
}

/*
 * del (The code below was used (copied) for delink() above)
 *
 * Removes node from the list.
 * Returns a pointer to the first next node that is still in the chain,
 * or nullptr if there is no next node (used for running over all nodes
 * in a loop that (can) remove(s) them).
 */
SbllNodeImpl* SbllNodeImpl::del()
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "SbllNodeImpl::del() " << this);
  if (magicnumber == REMOVEDNODE)
    DoutFatal(dc::core, "Trying to remove a node twice");
  if (magicnumber != MAGICNODE)
    DoutFatal(dc::core, "Trying to remove a non-existant node!");
#endif

  /* Is this node already in the process of being removed ? */
  if (get_state() == SBLL_STATE_BEING_REMOVED)
  {
#ifdef DEBUGLLISTS
    Dout(dc::llists, "This node (" << this << ") is already in the process of being removed");
#endif
    for(SbllNodeImpl* p = next; p; p = p->next)
      if (p->get_state() == SBLL_STATE_IN_USE)
	return p;		// Return first node that is not being removed
    return nullptr;		// No nodes left
  }
  set_state(SBLL_STATE_BEING_REMOVED);

  done();			// Call user function
  				// This is now allowed to call del()
  				// for any node, even nodes that are
  				// in the process of being removed.

  /* Find the next non-removed node if any */
  SbllNodeImpl* nextnode;
  for(nextnode = next; nextnode; nextnode = nextnode->next)
    if (nextnode->get_state() == SBLL_STATE_IN_USE)
      break;

  /* Delink this node from the chain: */
  if (prev()) // Didn't we just remove the list itself ?
  {
    if ((prev()->next = next))
      next->set_prev(prev());
  }
  else if (next)
  {
#ifdef CWDEBUG
    if (next->get_state() != SBLL_STATE_BEING_REMOVED)
      DoutFatal(dc::core, "'next' node not removed, but list is gone!?");
#endif
    next->set_prev(0); // Tell the next node too that the list is gone.
  }

  Dout(dc::llists, "this = " << this << "; deleting " << this << " (this)");
#if CWDEBUG_ALLOC && defined(DEBUGLLISTS)
  if (!libcwd::find_alloc(this))
  {
    Dout(dc::llists|continued_cf, "Trying to delete a node");
    print_whoami(this);
    Dout(dc::finish, " that was not allocated with new.");
    DoutFatal(dc::core, "Use 'new' to allocate nodes.");
  }
#endif
  // If you coredump here, you probably did define a node without using 'new'.
  // Using DEBUGMALLOC should detect this too though.
  delete this;
  return (SbllNodeImpl*)nextnode;
}

/*
 * find
 *
 * Finds the node that matches 'key' (using find_cmp()).
 */
SbllNodeImpl* SbllListImpl::find(SbllNodeImpl* start_node, void const* key) const
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "this = " << this << " (\"" << name << "\"); SbllListImpl::find(" << key << ")");
  if (magicnumber != MAGICLIST)
    DoutFatal(dc::core, "Not a list");
  if (start_node)
  {
    if (start_node->magicnumber != MAGICNODE)
      DoutFatal(dc::core, "start_node has wrong magicnumber!");
    SbllNodeImpl* node = start_node;
    while (node != next)
    {
      if (node->prev()->magicnumber == MAGICLIST)
	DoutFatal(dc::core, "start_node is not a node of " << name << " but of " << ((SbllListImpl*)node->prev())->name);
      node = (SbllNodeImpl*)node->prev();
      if (node->magicnumber != MAGICNODE)
        DoutFatal(dc::core, "start_node in corrupted chain");
    }
  }
  else if (next)
    DoutFatal(dc::core, "start_node NULL, but list (\"" << name << "\") not empty!");
#else
  Dout(dc::llists, "this = " << this << "; SbllListImpl::find()");
#endif
  for(SbllNodeImpl* node = start_node ? start_node : next; node; node = node->next)
  {
    if (node->get_state() == SBLL_STATE_BEING_REMOVED)
    {
#ifdef DEBUGLLISTS
      Dout(dc::warning, "Doing a find() in a list with nodes that are in the process of being removed!");
#endif
      continue;
    }
    if (node->find_cmp(key))
      return node;
  }
  return nullptr;
}

void SbllListImpl::remove_all_nodes()
{
#ifdef DEBUGLLISTS
  Dout(dc::llists, "Removing list \"" << name << "\" which still has nodes");
#else
  Dout(dc::llists, "Removing list that still has nodes");
#endif
  SbllNodeImpl* node = next;
  while(node)
    node = node->del();
}

SbllListImpl::~SbllListImpl()
{
#ifdef DEBUGLLISTS
  if (magicnumber != MAGICLIST)
    DoutFatal(dc::core, "Destructing an invalid list");
#endif
  Dout(dc::llists|continued_cf, "this = " << this << "; ~SbllListImpl()");
#ifdef DEBUGLLISTS
  Dout(dc::continued, " (\"" << name << "\")");
#endif
  Dout(dc::finish, "");
  if (next)
  {
    remove_all_nodes();
    if (next)			// Was there a node in SBLL_STATE_BEING_REMOVED ?
      next->set_prev(0);	// Inform this node that it's list is gone!
  }
#ifdef DEBUGLLISTS
  magicnumber = REMOVEDLIST;
#endif
}

#ifdef DEBUGLLISTS
void SbllListImpl::check_consistency()
{
  if (magicnumber != MAGICLIST)
    DoutFatal(dc::core, "Not a list");
  if (next && next->prev() != this)
    DoutFatal(dc::core, "Corrupted links");
  for(SbllNodeImpl* node = next; node; node = node->next)
  {
    if (node->prev()->next != node)
      DoutFatal(dc::core, "Corrupted links");
  }
}

void SbllListImpl::showlist()
{
  Dout(dc::notice|continued_cf, name << ": " << next);
  SbllNodeImpl* node = next;
  while (node)
  {
    node = node->next;
    Dout(dc::continued, " -> " << node);
  }
  Dout(dc::finish, "");
}
#endif

} // namespace utils
