#ifndef GITITEM_H
#define GITITEM_H

#include <type_traits>

#include "memory"
#include "GitDeleters.h"

namespace details
{

// helper alias to conveniently determine unique_ptr base for GitItem
template< class T >
using unique_ptr_base = std::unique_ptr< T, type_deleter< T > >;

template< typename T >
struct tag_checker { using type = void; };

template< typename T >
using tag_checker_t = typename tag_checker< typename T::_internalType >::type;

} //details

namespace git_handler
{

namespace item
{

// Base item class
class item
{
public:
    virtual ~item() = default;
};

template< class LibGitItemType >
class git_item : public item, public details::unique_ptr_base< LibGitItemType >
{    
    using details::unique_ptr_base< LibGitItemType >::unique_ptr;

public:
    using _internalType = LibGitItemType;
};

// type checkers
template< typename T, typename = void, typename = void >
struct is_git_item : std::false_type{};

template< typename T >
struct is_git_item< T, details::tag_checker_t< T >, std::enable_if_t< std::is_base_of< item, T >::value > > : std::true_type{};

// Item types
enum class type;

} //item

}//git_handler

#endif
