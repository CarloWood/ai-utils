#pragma once

#include <vector>
#include <functional>

// Register global (POD) types and do callbacks per object once main is reached.
//
// Usage:
//
// Consider having a global object of type G, but it being
// a constexpr because you want it to be static initialized
// and used in headers:
//
//     // .h
//     static constexpr G g1 = { 1, "initialization here" };
//
// This object has no constructor and might have many "copies",
// since it is in a header file.
//
// You want for each distinct G (g1, g2, g3, ...) to have the static callback
// function called `G::register(size_t n, G const& g, char const* desc)`
// where `n` is total number of G objects (hence the total number of callbacks
// that will happen), `g` is the current G object and `desc` is a description
// (an arbitrary argument).
//
//     //static
//     void G::register(size_t total_number_of_Gs, G const& g, char const* desc)
//     { ... }
//
// Registration needs to happen in a .cxx file. One registration per global object.
// This is done by creating global utils::Register<G> objects, with dynamic
// initialization, that is passed a lambda that takes a size_t. For example,
//
//     // .cxx
//     namespace {
//     utils::Register<G> g1_([](size_t n){ G::register(n, g1, "g1"); });
//     } // namespace
//
// Finally, somewhere at the top of main you need to add
//
//     int main()
//     {
//       Debug(debug::init());
//       utils::RegisterGlobals::finish_registration();         // Add this.
//       ...
//
namespace utils {

class RegisterGlobals
{
 private:
  // Ths must be a POD type: it is used before it can be dynamically constructed.
  static RegisterGlobals* s_list_start;
  RegisterGlobals* m_next;

 protected:
  void add(RegisterGlobals* category)
  {
    category->m_next = s_list_start;
    s_list_start = category;
  }

 public:
  RegisterGlobals() = default;

  // To be called from the start of main().
  static void finish_registration()
  {
    for (RegisterGlobals* element = s_list_start; element; element = element->m_next)
      element->finish();
  }

 private:
  virtual void finish() = 0;
};

template<typename T>
class Register : public RegisterGlobals
{
 private:
  static std::vector<std::function<void(size_t)>> s_global_objects;

 public:
  Register(std::function<void(size_t)> callback)
  {
    if (s_global_objects.empty())
      RegisterGlobals::add(this);
    s_global_objects.emplace_back(std::move(callback));
  }

 private:
  static void do_finish()
  {
    size_t const number_of_objects = s_global_objects.size();
    for (auto&& callback : s_global_objects)
      callback(number_of_objects);
    // Free resources.
    s_global_objects.clear();
    s_global_objects.shrink_to_fit();
  }

  void finish() override final { do_finish(); }
};

template<typename T>
std::vector<std::function<void(size_t)>> Register<T>::s_global_objects;

} // namespace utils
