#pragma once

#include <memory>
#include <atomic>
#include "utils/Badge.h"
#include "threadsafe/threadsafe.h"
#include "debug.h"

// threading::ObjectTracker
//
// Allows objects to have a non-moving tracker object, allocated on the heap,
// that manages a pointer that points back to them.
//
// This allows other objects to point to the tracker object without having
// to worry if the tracked object is moved in memory.
//
// Usage:
#if -0 // EXAMPLE_CODE

// Forward declare the to-be-tracked object.
class Node;

// Define a corresponding tracker class by deriving from utils::ObjectTracker, passing the type of the tracked object.
class NodeTracker : public utils::ObjectTracker<Node>
{
 public:
  // All trackers must take a Badge as first argument that allows utils::TrackedObject to construct them.
  // The second argument must be a Node* which is passed to ObjectTracker base class.
  NodeTracker(
      utils::Badge<utils::TrackedObject<NodeTracker>>,
      Node* tracked) :
    utils::ObjectTracker<Node>(tracked) { }
};

// Or simply use:
//using NodeTracker = utils::ObjectTracker<Node>;

// The to-be-tracked object must be derived from utils::TrackedObject, passing the tracker class.
class Node : public utils::TrackedObject<NodeTracker>
{
 private:
  std::string s_;

 public:
  Node(std::string const& s) : s_(s) { }

  std::string const& s() const { return s_; }
};

int main()
{
  // Now one can construct a Node object:
  Node node("hello");
  // And obtain the tracker at any moment (also after it was moved):
  std::weak_ptr<NodeTracker> node_tracker = node;

  // Even if node is moved,
  Node node2(std::move(node));
  // node_tracker will point to node2:
  std::cout << "s = " << node_tracker.lock()->tracked_object().s() << std::endl;  // Prints "hello".
}
#endif // EXAMPLE_CODE

namespace utils::threading {

template<typename Tracker>
class TrackedObject;

template<typename TrackedUnlocked>
requires utils::is_specialization_of_v<TrackedUnlocked, threadsafe::Unlocked>
class ObjectTracker
{
 public:
  using tracked_unlocked_type = TrackedUnlocked;
  using tracked_type = typename tracked_unlocked_type::data_type;
  using policy_type = typename tracked_unlocked_type::policy_type;

 protected:
  std::atomic<tracked_unlocked_type*> tracked_unlocked_ptr_;

  // Used by trackers that are derived from ObjectTracker.
  ObjectTracker(TrackedUnlocked* tracked_unlocked_ptr) : tracked_unlocked_ptr_(tracked_unlocked_ptr) { }

 public:
  // Construct a new ObjectTracker that tracks the object pointed to by tracked_unlocked_ptr.
  ObjectTracker(utils::Badge<TrackedObject<ObjectTracker>>, TrackedUnlocked* tracked_unlocked_ptr) :
    tracked_unlocked_ptr_(tracked_unlocked_ptr) { }

  // This is called when the object is moved in memory, see below.
  void set_tracked_unlocked(utils::Badge<TrackedObject<ObjectTracker>>, tracked_unlocked_type* tracked_unlocked_ptr)
  {
    // The tracked_unlocked_type object that tracked_unlocked_ptr points to must be write-locked (blocking all readers (and writers))!
    // Note that we can use relaxed memory order because we are inside the critical area of *tracked_unlocked_ptr anyway.
    tracked_unlocked_ptr_.store(tracked_unlocked_ptr, std::memory_order::relaxed);
  }

  // Accessors.
  tracked_unlocked_type const& tracked_unlocked() const { return *tracked_unlocked_ptr_; }
  tracked_unlocked_type& tracked_unlocked() { return *tracked_unlocked_ptr_; }

  // Automatic conversion to a tracked_unlocked_type reference.
  operator tracked_unlocked_type const&() const { return *tracked_unlocked_ptr_; }
  operator tracked_unlocked_type&() { return *tracked_unlocked_ptr_; }
};

template<typename Tracker>
requires utils::is_specialization_of_v<Tracker, ObjectTracker>
class TrackedObject
{
 protected:
  std::shared_ptr<Tracker> tracker_;

  TrackedObject() : tracker_(std::make_shared<Tracker>(utils::Badge<TrackedObject>{}, static_cast<typename Tracker::tracked_unlocked_type*>(this)))
  {
  }

  TrackedObject(TrackedObject&& orig) : tracker_(std::move(orig.tracker_))
  {
    // External synchronization is required: it is not allowed to move a TrackedObject
    // while concurrently accessing the same object. No synchronization is performed here.
    // Typically one should work with a threadsafe::Unlocked<Foo, ...> where Foo is derived
    // from this utils::threading::TrackedObject<FooTracker> and where FooTracker is derived
    // from utils::threading::ObjectTracker<threadsafe::Unlocked<Foo, ...>>.
    tracker_->set_tracked_unlocked({}, static_cast<typename Tracker::tracked_unlocked_type*>(this));
  }

  ~TrackedObject()
  {
    if (tracker_) // This is null if the tracked object was moved.
      tracker_->set_tracked_unlocked({}, nullptr);
  }

 public:
  // Accessor for the Tracker object. Make sure to keep the TrackedObject alive while using this.
  Tracker const& tracker() const
  {
    // Note that tracker_ can only be null when the Tracker was moved.
    // Do not call this function (or any other member function except the destructor) on a moved object!
    ASSERT(tracker_);
    return *tracker_;
  }

  Tracker& tracker()
  {
    // See above.
    ASSERT(tracker_);
    return *tracker_;
  }

  // Automatic conversion to a weak_ptr.
  operator std::weak_ptr<Tracker>() const { return tracker_; }
};

} // namespace utils::threading
