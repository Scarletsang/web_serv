#include "AllowMethods.hpp"

#include <cassert>
#include <iostream>

#include <bitset>

#include "Configuration/Directive.hpp"

namespace directive
{
  AllowMethods::AllowMethods()
    : Directive(), accepted_methods_(AllowMethods::kDefaultMethods) {}

  AllowMethods::AllowMethods(const Context& context)
    : Directive(context), accepted_methods_(AllowMethods::kDefaultMethods) {}

  AllowMethods::AllowMethods(const AllowMethods& other)
    : Directive(other), accepted_methods_(other.accepted_methods_) {}

  AllowMethods& AllowMethods::operator=(const AllowMethods& other)
  {
    if (this != &other)
    {
      Directive::operator=(other);
      accepted_methods_ = other.accepted_methods_;
    }
    return *this;
  }

  AllowMethods::~AllowMethods() {}

  bool AllowMethods::is_block() const
  {
    return false;
  }

  Directive::Type AllowMethods::type() const
  {
    return Directive::kDirectiveAllowMethods;
  }

  void  AllowMethods::print(int) const
  {
    if (accepted_methods_ == 0)
	{
	  std::cout << "NONE";
	}
    else
	{
      if (accepted_methods_ & kMethodGet)
	    std::cout << "GET ";
      if (accepted_methods_ & kMethodPost)
	    std::cout << "POST ";
      if (accepted_methods_ & kMethodDelete)
	    std::cout << "DELETE";
	}
  }

  void AllowMethods::set(int method)
  {
    assert(method >= 0 && method < 8);
    accepted_methods_ = method;
  }

  Methods AllowMethods::get() const
  {
    return accepted_methods_;
  }

  bool  AllowMethods::is_allowed(Method method) const
  {
    return (accepted_methods_ & method) != 0;
  }
} // namespace configuration
