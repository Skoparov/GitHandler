#ifndef GITITEMFACTORY_H
#define GITITEMFACTORY_H

#include <map>

#include "GitItem.h"
#include "details/UniquePointerCast.h"

namespace git_handler
{

namespace factory
{

// Generalized factory return value
using ItemPtr = std::unique_ptr< item::Item >;

//////////////////////////////////////////////////////////////////////////////
///////////////           GitItemFactoryInterface       //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitItemFactoryIf
{
public:
    virtual ~GitItemFactoryIf() = default;
    virtual ItemPtr create() const = 0;
};

//////////////////////////////////////////////////////////////////////////////
///////////////               GitItemFactory            //////////////////////
//////////////////////////////////////////////////////////////////////////////

template < class LibGitItemType >
class GitItemFactory : public GitItemFactoryIf
{
    using Deleter = details::TypeDeleter< LibGitItemType >;
    using CurrItem = item::GitItem< LibGitItemType >;

public:
    ItemPtr create() const override
    {
        LibGitItemType* item = nullptr;
        return std::make_unique< CurrItem >( item, mDeleter );
    }

private:
    Deleter mDeleter{ deleters::deleteItem< LibGitItemType > };
};

//////////////////////////////////////////////////////////////////////////////
///////////////                GitItemCreator           //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitItemCreator
{
    using FactoryStorage = std::map< item::Type, std::unique_ptr< GitItemFactoryIf > >;

public:
    GitItemCreator(){}
    GitItemCreator( const GitItemCreator& ) = delete;
    GitItemCreator( GitItemCreator&& ) = delete;
    GitItemCreator& operator=( const GitItemCreator& ) = delete;
    GitItemCreator& operator=( GitItemCreator&& ) = delete;

    template < class LibGitItemType >
    bool registerItemType( const item::Type itemType, std::unique_ptr< GitItemFactoryIf >&& factory )
    {
        if ( !mFactories.count( itemType ) )
        {
            mFactories.emplace( itemType, std::move( factory ) );
            return true;
        }

        return false;
    }

    template< class GitItemType >
    std::unique_ptr< GitItemType > create( const item::Type itemType, typename GitItemType::_internalType* manage = nullptr )
    {
        auto factory = mFactories.find( itemType );

        if ( factory != mFactories.end() )
        {
            auto result = details::dynamic_unique_pointer_cast< GitItemType >( factory->second->create() );
            result->reset( manage );
            return result;
        }

        return nullptr;
    }

    static GitItemCreator& get()
    {
        static GitItemCreator instance;
        return instance;
    }

private:
    FactoryStorage mFactories;
};

}// factory

}//git_handler

#endif
