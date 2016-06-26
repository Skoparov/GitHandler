#ifndef GITITEM_H
#define GITITEM_H

#include "memory"
#include "GitDeleters.h"

namespace details
{

// helper alias to conveniently determine unique_ptr base for GitItem
template< class T >
using UniquePtrBase = std::unique_ptr< T, TypeDeleter< T > >;

} //details

namespace git_handler
{

namespace item
{

// Base item class
class Item
{
public:
    virtual ~Item() = default;
};

template< class LibGitItemType >
class GitItem : public Item, public details::UniquePtrBase< LibGitItemType >
{    
    using details::UniquePtrBase< LibGitItemType >::unique_ptr;

public:
    using _internalType = LibGitItemType;
};

// Item types
enum class Type;

} //item

}//git_handler

#endif
