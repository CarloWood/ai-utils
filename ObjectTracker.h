#pragma once

#include <memory>
#include "utils/Badge.h"
#include "debug.h"

// ObjectTracker
//
// Allows objects to have a non-moving tracker object, allocated on the heap,
// that manages a pointer that points back to them.
//
// This allows other objects to point to the tracker object without having
// to worry if the tracked object is moved in memory.
//
// Note: this version is NOT thread-safe!
// If you need concurrent access, use utils/threading/ObjectTracker.h instead.
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

namespace utils {

template<typename Tracker>
class TrackedObject;

template<typename Tracked>
class ObjectTracker
{
 public:
  using tracked_type = Tracked;

 protected:
  Tracked* tracked_object_ptr_;

  // Used by trackers that are derived from ObjectTracker.
  ObjectTracker(Tracked* tracked_object_ptr) : tracked_object_ptr_(tracked_object_ptr) { }

 public:
  // Construct a new ObjectTracker that tracks the object pointed to by tracked_object_ptr.
  ObjectTracker(utils::Badge<TrackedObject<ObjectTracker>>, Tracked* tracked_object_ptr) : tracked_object_ptr_(tracked_object_ptr) { }

  // This is called when the object is moved in memory, see below.
  void set_tracked_object(Tracked* tracked_object_ptr)
  {
    tracked_object_ptr_ = tracked_object_ptr;
  }

  // Accessors.
  Tracked const& tracked_object() const { return *tracked_object_ptr_; }
  Tracked& tracked_object() { return *tracked_object_ptr_; }

  // Automatic conversation to a Tracked reference.
  operator Tracked const&() const { return *tracked_object_ptr_; }
  operator Tracked&() { return *tracked_object_ptr_; }
};

template<typename Tracker>
class TrackedObject
{
 protected:
  std::shared_ptr<Tracker> tracker_;

  TrackedObject() : tracker_(std::make_shared<Tracker>(utils::Badge<TrackedObject>{}, static_cast<typename Tracker::tracked_type*>(this)))
  {
  }

  TrackedObject(TrackedObject&& orig) : tracker_(std::move(orig.tracker_))
  {
    // External synchronization is required: it is not allowed to move a TrackedObject
    // while concurrently accessing the same object. No synchronization is performed here.
    tracker_->set_tracked_object(static_cast<typename Tracker::tracked_type*>(this));
  }

  ~TrackedObject()
  {
    if (tracker_) // This is null if the tracked object was moved.
      tracker_->set_tracked_object(nullptr);
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

} // namespace utils
