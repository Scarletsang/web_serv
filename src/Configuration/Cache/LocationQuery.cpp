#include "LocationQuery.hpp"

#include <cassert>

#include <string>
#include <vector>
#include <limits>

#include "constants.hpp"
#include "Configuration/Directive.hpp"
#include "Configuration/Directive/Block.hpp"
#include "Configuration/Directive/Block/Location.hpp"
#include "Configuration/Directive/Block/Server.hpp"
#include "Configuration/Directive/Simple.hpp"
#include "Configuration/Directive/Simple/MimeTypes.hpp"
#include "Configuration/Directive/Simple/AllowMethods.hpp"
#include "Configuration/Directive/Simple/Return.hpp"
#include "Configuration/Directive/Simple/Cgi.hpp"

namespace cache
{

  ////////////////////////////////////////////////
  ////////////   LocationQueryCache   ////////////
  ////////////////////////////////////////////////

  LocationQuery::LocationQuery()
    : server_block(NULL),
      match_path(),
      allowed_methods(),
      client_max_body_size(0),
      redirect(NULL),
      cgis(),
      root(),
      indexes(),
      autoindex(false),
      mime_types(),
      error_pages(),
      access_log(),
      error_log() {}

  void  LocationQuery::construct(const directive::ServerBlock* server_block_, const directive::LocationBlock* location_block)
  {
    assert(server_block_ != NULL);
    server_block = server_block_;
    const directive::DirectiveBlock*  target_block = location_block;
    if (target_block)
      construct_match_path(location_block);
    else
      target_block = server_block;
    construct_allowed_methods(target_block);
    construct_client_max_body_size(target_block);
    construct_return(target_block);
    construct_cgis(target_block);
    construct_root(target_block);
    construct_indexes(target_block);
    construct_autoindex(target_block);
    construct_mime_types(target_block);
    construct_error_pages(target_block);
    construct_access_log(target_block);
    construct_error_log(target_block);
  }

  void  LocationQuery::construct_match_path(const directive::LocationBlock* location_block)
  {
    match_path = location_block->match();
  }

  void  LocationQuery::construct_allowed_methods(const directive::DirectiveBlock* target_block)
  {
    const directive::AllowMethods* directive = 
      static_cast<const directive::AllowMethods*>(closest_directive(target_block, Directive::kDirectiveAllowMethods));

    allowed_methods = directive ? directive->get() : constants::kDefaultAllowedMethods;
  }

  void  LocationQuery::construct_client_max_body_size(const directive::DirectiveBlock* target_block)
  {
    const directive::ClientMaxBodySize* directive = 
      static_cast<const directive::ClientMaxBodySize*>(closest_directive(target_block, Directive::kDirectiveClientMaxBodySize));
 
    client_max_body_size = directive ? directive->get() : constants::kDefaultClientMaxBodySize; // 1MB
  }

  void  LocationQuery::construct_return(const directive::DirectiveBlock* target_block)
  {
    redirect = static_cast<const directive::Return*>(closest_directive(target_block, Directive::kDirectiveReturn));
  }

  void  LocationQuery::construct_cgis(const directive::DirectiveBlock* target_block)
  {
    std::vector<const Directive*> cgis_directives = collect_directives(target_block, Directive::kDirectiveCgi, &CheckCgi);
    cgis.reserve(cgis_directives.size());
    for (std::vector<const Directive*>::const_iterator it = cgis_directives.begin(); it != cgis_directives.end(); ++it)
    {
      cgis.push_back(static_cast<const directive::Cgi*>(*it));
    }
  }

  void  LocationQuery::construct_root(const directive::DirectiveBlock* target_block)
  {
    const directive::Root* directive = 
      static_cast<const directive::Root*>(closest_directive(target_block, Directive::kDirectiveRoot));

    root = directive ? directive->get() : constants::kDefaultRoot;
  }

  void  LocationQuery::construct_indexes(const directive::DirectiveBlock* target_block)
  {
    std::vector<const Directive*> indexes_directives = collect_directives(target_block, Directive::kDirectiveIndex, &CheckIndex);
    if (indexes_directives.empty())
    {
      indexes.push_back(&constants::kDefaultIndex);
      return ;
    }
    indexes.reserve(indexes_directives.size());
    for (std::vector<const Directive*>::const_iterator it = indexes_directives.begin(); it != indexes_directives.end(); ++it)
    {
      indexes.push_back(static_cast<const directive::Index*>(*it));
    }
  }

  void  LocationQuery::construct_autoindex(const directive::DirectiveBlock* target_block)
  {
    const directive::Autoindex* directive = 
      static_cast<const directive::Autoindex*>(closest_directive(target_block, Directive::kDirectiveAutoindex));
    
    autoindex = directive ? directive->get() : constants::kDefaultAutoindex;
  }

  void  LocationQuery::construct_mime_types(const directive::DirectiveBlock* target_block)
  {
    mime_types = static_cast<const directive::MimeTypes*>(closest_directive(target_block, Directive::kDirectiveMimeTypes));
    if (!mime_types)
      mime_types = &constants::kDefaultMimeTypes;
  }

  void  LocationQuery::construct_error_pages(const directive::DirectiveBlock* target_block)
  {
    std::vector<const Directive*> error_pages_directives = collect_directives(target_block, Directive::kDirectiveErrorPage, &CheckErrorPage);
    error_pages.reserve(error_pages_directives.size());
    for (std::vector<const Directive*>::const_iterator it = error_pages_directives.begin(); it != error_pages_directives.end(); ++it)
    {
      error_pages.push_back(static_cast<const directive::ErrorPage*>(*it));
    }
  }

  void  LocationQuery::construct_access_log(const directive::DirectiveBlock* target_block)
  {
    const directive::AccessLog* directive = 
      static_cast<const directive::AccessLog*>(closest_directive(target_block, Directive::kDirectiveAccessLog));

    access_log = directive ? directive->get() : constants::kDefaultAccessLog;
  }

  void  LocationQuery::construct_error_log(const directive::DirectiveBlock* target_block)
  {
    const directive::ErrorLog* directive = 
      static_cast<const directive::ErrorLog*>(closest_directive(target_block, Directive::kDirectiveErrorLog));

    error_log = directive ? directive->get() : constants::kDefaultErrorLog;
  }

  const Directive*  LocationQuery::closest_directive(const directive::DirectiveBlock* block, Directive::Type type)
  {
    assert(block != NULL);
    const Directive*  result = NULL;
    int index = std::numeric_limits<int>::max();
    while (block)
    {
      directive::DirectivesRange query_result = block->query_directive(type);
      if (directive::DirectiveRangeIsValid(query_result))
      {
        // Find the last directive with index less than the current block
        for (directive::DirectivesRange::first_type it = query_result.first; it != query_result.second; ++it)
        {
          const Directive* directive = it->second;
          if (directive->index() < index)
            result = directive;
          else
            break;
        }
        if (result)
          break;
      }
      index = block->index();
      block = block->parent();
    }
    return result;
  }

  std::vector<const Directive*>  LocationQuery::collect_directives(const directive::DirectiveBlock* block,
                                                                   Directive::Type type,
                                                                   DuplicateChecker is_duplicated)
  {
    std::vector<const Directive*> result;
    int index = std::numeric_limits<int>::max();
    while (block)
    {
      directive::DirectivesRange query_result = block->query_directive(type);
      if (directive::DirectiveRangeIsValid(query_result))
      {
        directive::DirectivesRange::second_type it = query_result.second;
        // The second iterator can be an end iterator, so we need to decrement it
        do {
          it--;
          const Directive* directive = it->second;
          if (directive->index() < index)
          {
            // If a directive is duplicated, only the first one is kept, so do nothing
            bool  is_duplicate = false;
            std::vector<const Directive*>::iterator result_it = result.begin();
            for (; result_it != result.end(); ++result_it)
            {
              if (is_duplicated(result_it, directive))
              {
                is_duplicate = true;
                break;
              }
            }
            // otherwise, add it to the result
            if (!is_duplicate)
              result.push_back(directive);
          }
        }
        while (it != query_result.first);
      }
      index = block->index();
      block = block->parent();
    }
    return result;
  }

  bool  CheckCgi(std::vector<const Directive*>::iterator directive_it, const Directive* new_directive)
  {
    const directive::Cgi* directive = static_cast<const directive::Cgi*>(new_directive);
    return static_cast<const directive::Cgi*>(*directive_it)->get() == directive->get();
  }

  bool  CheckIndex(std::vector<const Directive*>::iterator directive_it, const Directive* new_directive)
  {
    const directive::Index* directive = static_cast<const directive::Index*>(new_directive);
    return static_cast<const directive::Index*>(*directive_it)->get() == directive->get();
  }

  bool  CheckErrorPage(std::vector<const Directive*>::iterator directive_it, const Directive* new_directive)
  {
    const directive::ErrorPage* directive = static_cast<const directive::ErrorPage*>(new_directive);
    return static_cast<const directive::ErrorPage*>(*directive_it)->get() == directive->get();
  }
} // namespace caches
