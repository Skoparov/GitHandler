#ifndef UNIQUE_POINTER_CAST_H
#define UNIQUE_POINTER_CAST_H

#include <memory>

namespace details
{
template< typename T_Dst, typename T_Src >
auto dynamic_unique_pointer_cast( std::unique_ptr< T_Src >&& src) -> std::unique_ptr< T_Dst >
{
    if( !src )
    {
        return nullptr;
    }

    T_Dst* ptr = dynamic_cast< T_Dst* >( src.get() );
    if( !ptr )
    {
        return nullptr;
    }

    src.release();
    std::unique_ptr< T_Dst > dst( ptr );

    return dst;
}

template< typename T_Dst, typename T_Src, typename Deleter >
auto dynamic_unique_pointer_cast( std::unique_ptr< T_Src, Deleter >&& src) -> std::unique_ptr< T_Dst, Deleter >
{
    if( !src )
    {
        return nullptr;
    }

    T_Dst* ptr = dynamic_cast< T_Dst* >( src.get() );
    if( !ptr )
    {
        return nullptr;
    }

    src.release();
    std::unique_ptr< T_Dst, Deleter > dst( ptr, std::move( src.get_deleter() ) );

    return dst;
}
}

#endif // UNIQUE_POINTER_CAST_H
