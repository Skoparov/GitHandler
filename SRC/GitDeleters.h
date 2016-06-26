#ifndef GITDELETERS_H
#define GITDELETERS_H

#include "git2.h"

namespace git_handler
{

namespace deleters
{

template< class TypeToDelete >
void deleteItem( TypeToDelete* );

}//deleters

}//git_handler

namespace details
{

// helper alias to conveniently determine a deleter type
template< class TypeToDelete >
using TypeDeleter = void( * )( TypeToDelete* );

}

#endif //GITDELETERS_H
