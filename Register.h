#pragma once

#include <vector>

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
  virtual ~RegisterGlobals() = default;

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
 public:
  using call_back_t = void (*)(T const&);

 private:
  static std::vector<std::pair<call_back_t, T const&>> s_global_objects;

 public:
  Register(call_back_t call_back, T const& global_object)
  {
    if (s_global_objects.empty())
      RegisterGlobals::add(this);
    s_global_objects.push_back({call_back, global_object});
  }

 private:
  static void do_finish()
  {
    for (auto&& p : s_global_objects)
    {
      p.first(p.second);
    }
  }

  void finish() override final { do_finish(); }
};

template<typename T>
std::vector<std::pair<typename Register<T>::call_back_t, T const&>> Register<T>::s_global_objects;

} // namespace utils
