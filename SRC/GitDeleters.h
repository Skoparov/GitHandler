#ifndef GITDELETERS_H
#define GITDELETERS_H

#include <git2.h>

namespace git_handler
{

namespace deleters
{

template< class TypeToDelete >
void delete_item( TypeToDelete* );

}//deleters

}//git_handler

namespace details
{

// helper alias to conveniently determine a deleter type
template< class TypeToDelete >
using type_deleter = void( * )( TypeToDelete* );

}

#endif //GITDELETERS_H
