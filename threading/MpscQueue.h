#pragma once

// Translated from http://www.1024cores.net/home/lock-free-algorithms/queues/intrusive-mpsc-node-based-queue
// into modern C++ by Carlo Wood (c) 2021.
//
// For this reason this file has the following license.
//
// Copyright (c) 2010-2011 Dmitry Vyukov. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
//
//   1. Redistributions of source code must retain the above copyright notice, this list of
//      conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above copyright notice, this list
//      of conditions and the following disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY DMITRY VYUKOV "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL DMITRY VYUKOV OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
// BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
//
// The views and conclusions contained in the software and documentation are those of
// the authors and should not be interpreted as representing official policies,
// either expressed or implied, of Dmitry Vyukov.
//
//
// All comments and memory orders used are Copyright (c) 2021 Carlo Wood,
// and also released under the same license.

#include <atomic>

namespace utils::threading {

struct MpscNode
{
  std::atomic<MpscNode*> m_next;
};

// MpscQueue
//
// Empty list:
//
//   m_tail ---> m_stub ===> nullptr
//                  ^
//                  |
//                m_head
//
// where '--->' means "points to the node",
// and '===>' "is a node with an m_next pointer that points to".
//
// Apply one push on an empty list:
//
//   m_tail ---> m_stub ===> node1 ===> nullptr
//                             ^
//                             |
//                           m_head
//
// A list with one or more nodes:
//
//   m_tail ---> [m_stub ===>] node1 ===> node2 ===> ... ===> nodeN ===> nullptr
//                                                              ^
//                                                              |
//                                                            m_head
//
// If no pop was performed since the last time the list was empty,
// then m_tail points to m_stub. If one or more pop's where performed
// not leading to an empty list then m_tail points to the next node
// that will be popped.
//
// Because the last line in push fixes the m_next pointer that m_head was
// pointing at (from nullptr to the new node), a list that has a few nodes
// pushed concurrently can also look like this:
//
//   m_tail ---> [m_stub ===>] node1 ===> node2 ===> ... ===> nodeK ===> nullptr, nodeL ===> nullptr, nodeM ===> nullptr, nodeN ===> nullptr
//                                                                                                                          ^
//                                                                                                                          |
//                                                                                                                        m_head
// Where the m_next pointers of nodes K, L and M are filled in "later" in any order, to end as
//
//   m_tail ---> [m_stub ===>] node1 ===> node2 ===> ... ===> nodeK ===> nodeL ===> nodeM ===> nodeN ===> nullptr
//                                                                                               ^
//                                                                                               |
//                                                                                             m_head
//
// Hence, trying to pop a node that was already pushed might fail (returning nullptr) if
// not all previous pushes also finished.
//
// This half finished list might get repaired in any order, so it might already be partially
// linked. Also, due to concurrent execution of pop on a list with one element, and multiple pushes,
// the most general unfinished state of the list might look something like:
//
// m_tail ---> node3 [===> node4...] ===> nullptr, node5 [===> node6...] ===> nullptr, ..., nodeN ===> nullptr
//                                                                                            ^
//                                                                                            |
//                                                                                          m_head
//
// where any of the nodes node4 or higher can be m_stub. Combining that with the
// previous possibility where m_tail still points to m_stub from the beginning,
// m_tail basically is a pointer to a possibly yet incompletely linked list of
// nodes where zero or one node might be m_stub (which should be skipped when
// encountered). Except for an empty list, because there has to be always at
// least one node in the list; so in that case m_stub is always there.
//
//
// Memory Order
// ------------
//
// Sequential-consistent ordering isn't needed: the producers, that call push,
// only store (to m_next); they load the value of m_head but that value is
// only written to by the std::atomic_exchange at the same time. This Read-
// Modify-Write operation has a total ordering anyway.
//
// The consumer, that calls pop, is the only thread (or multiple threads
// where a total ordering is already enforced and only one at a time calls
// pop). So, there is no modification order that could be different between
// two or more threads anyway.
//
// In other words, the std::atomic_exchange in push() has a total ordering
// and determines the order in which nodes are pushed onto the queue.
// The consumer will pop nodes in that order.
//
// At the time of writing, release-consume ordering is still being discouraged
// (since C++17) as its specification is being revised, so that leaves
// release-acquire ordering.
//
// Whenever a pointer to a node is returned by pop, the data that was written
// to that node must be visible (you'd think that this is typical a release-consume
// case; but at least on Intel architectures it shouldn't make any difference).
// The returned value is tail, which is a local variable assigned from m_tail
// or next, where also m_tail was assigned from next, where next is loaded
// from m_next. In other words, we have to read m_next with memory order acquire,
// and the non-null store in push with memory order release.
//
// In pop() this only leaves the load of m_head.
// At first sight, it seems pointless to read m_head at all: immediately
// after loading it it could be changed again by any number of calls to push
// by other threads. However, after any call to push, m_head would at most
// be changed away from the value of tail, making the expression tail != head
// true. So, in terms of FuzzyBool (see utils/FuzzyBool.h) the construct:
//
//  MpscNode* head = m_head.load();
//  utils::FuzzyBool unequal = (tail != head) ? fuzzy::True : fuzzy::WasFalse;
//  if (unequal.is_true())
//    return nullptr;
//
// is appropriate.
//
// This means that the subsequent call to push(&m_stub) guarantees that
// `unequal` becomes (and stays) True, so that if the function subsequently
// returns nullptr and reenters, it will never call push(&m_stub) again.
// In turn this then guarantees that there is only one m_stub in the linked
// list at all times, which is important because it only has one m_next
// pointer, of course.
//
// The release-acquire ordering of m_next already guarantees that the
// value of m_head that is loaded isn't some insanely old value pointing
// to ages old values: it HAS to be the value written by the atomic_exchange
// just prior to the value that we loaded from m_next at the top of pop(),
// or any later value of course. It can not be an older value because that
// atomic_exchange store is visible, and RMW's have a total order, so the
// last value written will hide any previously written values.
//
// The call to pop() at this point could return nullptr and reenter at
// the top, get to the same point and load m_head again; this value is
// now guaranteed to be unequal to the last value of m_head, and therefore
// unequal to m_tail, because it is the very same thread that just stored
// a value to m_head (by calling push(&m_stub)).
//
// It is therefore perfectly fine to load the value of m_head with memory
// order relaxed.
//
// The store of the nullptr in push() can also be memory order relaxed,
// because we don't use anything that was written to memory before that
// store. In most cases, when the value of nullptr is read, pop() just
// returns nullptr. In the one case that it doesn't, it pushes m_stub
// and more or less starts over.
//
// Finally, the atomic_exchange can be relaxed too, since it is ordered
// with respect to itself anyway, and the only other instance where we
// read m_head that is loaded with memory order relaxed.
//
class MpscQueue
{
 protected:
  std::atomic<MpscNode*> m_head;
  std::atomic<MpscNode*> m_tail;
  MpscNode               m_stub;

 public:
  MpscQueue() : m_head(&m_stub), m_tail(&m_stub), m_stub{{nullptr}} { }

  void push(MpscNode* node)
  {
    node->m_next.store(nullptr, std::memory_order_relaxed);
    MpscNode* prev = m_head.exchange(node, std::memory_order_relaxed);
    // Here m_head points to the new node, which either points to null
    // or already points to the NEXT node that was pushed AND completed, etc.
    // Now fix the next pointer of the node that m_head was pointing at.
    prev->m_next.store(node, std::memory_order_release);
  }

  MpscNode* pop()
  {
    MpscNode* tail = m_tail.load(std::memory_order_relaxed);
    MpscNode* next = tail->m_next.load(std::memory_order_acquire);
    if (tail == &m_stub)
    {
      // If m_tail ---> m_stub ===> nullptr, at the time of the above load(), then return nullptr.
      // This is only the case for an empty list (or when the first push (determined by the first push
      // that performed the atomic_exchange) to an empty list, or the first push following the
      // push(&m_stub) (see below), didn't complete yet).
      if (nullptr == next)
        return nullptr;
      // Skip m_stub.
      m_tail.store(next, std::memory_order_relaxed);
      tail = next;
      next = tail->m_next.load(std::memory_order_acquire);
    }
    // Are there at least two nodes in the list?
    // Aka, m_tail ---> node1 ===> node2
    if (next)
    {
      // Remove node and return it.
      m_tail.store(next, std::memory_order_release);
      return tail;
    }
    // If we get here we had the situation, at the time of the above load(), of
    // m_tail ---> [m_stub ===>] node1 ===> nullptr and we now have
    // tail ---> node1; where, at least very recently, node1 ===> nullptr.
    MpscNode* head = m_head.load(std::memory_order_relaxed);
    // If head was changed in the meantime then a push is or was in progress
    // and we fail to read node1 for now. This is to make sure we won't push
    // m_stub onto the list when it is already there.
    if (tail != head)
      return nullptr;
    // Make sure we have at least two nodes again.
    push(&m_stub);
    // In the simplest case of no other races we now have:
    //
    //   tail ---> node1 ===> m_stub ===> nullptr
    //                           ^
    //                           |
    //                         m_head
    //
    // If there where other - not yet completed - pushes however,
    // we can have a situation like this:
    //
    //   tail --->  node1 ===> nullptr, node2 ===> node3 ===> nullptr, m_stub ===> nullptr
    //                                                                   ^
    //                                                                   |
    //                                                                 m_head
    // where node2 was (incompletely) pushed before we pushed &m_stub.
    // In that case this next will be null:
    next = tail->m_next.load(std::memory_order_acquire);
    if (next)
    {
      // Remove node and return it.
      m_tail.store(next, std::memory_order_relaxed);
      return tail;
    }
    return nullptr;
  }
};

} // namespace utils::threading
