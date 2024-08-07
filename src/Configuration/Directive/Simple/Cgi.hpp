#pragma once

#include <string>

#include "misc/Maybe.hpp"
#include "Configuration/Directive.hpp"

namespace directive
{
  class Cgi : public Directive
  {
    public:
      Cgi();
      Cgi(const Context& context);
      Cgi(const Cgi& other);
      Cgi& operator=(const Cgi& other);
      virtual ~Cgi();

      virtual bool        is_block() const;
      virtual Type        type() const;
	  virtual void		  print(int) const;

      void                set(const std::string& extension, const std::string& cgi_path);

      const std::string&  extension() const;
      const std::string&  cgi_path() const;
      const std::string&  get() const;

      Maybe<std::string>  match(std::string extension) const;

    private:
      std::string         extension_;
      std::string         cgi_path_;
  };
} // namespace configuration
