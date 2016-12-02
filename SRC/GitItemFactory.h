#ifndef GITITEMFACTORY_H
#define GITITEMFACTORY_H

#include <map>
#include <mutex>

#include "GitItem.h"
#include "details/UniquePointerCast.h"

namespace git_handler
{

namespace factory
{

// Generalized factory return value
using item_ptr = std::unique_ptr< item::item >;

//////////////////////////////////////////////////////////////////////////////
///////////////           GitItemFactoryInterface       //////////////////////
//////////////////////////////////////////////////////////////////////////////

class igit_item_factory
{
public:
    virtual ~igit_item_factory() = default;
    virtual item_ptr create() const = 0;
};

//////////////////////////////////////////////////////////////////////////////
///////////////               GitItemFactory            //////////////////////
//////////////////////////////////////////////////////////////////////////////

template < class LibGitItemType >
class git_item_factory : public igit_item_factory
{
    using deleter = details::type_deleter< LibGitItemType >;
    using curr_item = item::git_item< LibGitItemType >;

public:
    item_ptr create() const override
    {
        LibGitItemType* item{ nullptr };
        return std::make_unique< curr_item >( item, m_deleter );
    }

private:
    deleter m_deleter{ deleters::delete_item< LibGitItemType > };
};

//////////////////////////////////////////////////////////////////////////////
///////////////                GitItemCreator           //////////////////////
//////////////////////////////////////////////////////////////////////////////

class git_item_creator
{
    using factory_storage = std::map< item::type, std::unique_ptr< igit_item_factory > >;

private:
    git_item_creator() = default;

public:    
    git_item_creator( const git_item_creator& ) = delete;
    git_item_creator( git_item_creator&& ) = delete;
    git_item_creator& operator=( const git_item_creator& ) = delete;
    git_item_creator& operator=( git_item_creator&& ) = delete;

    template < class LibGitItemType >
    bool register_item_type( const item::type& item_type, std::unique_ptr< igit_item_factory >&& factory )
    {
        std::lock_guard< std::mutex > l{ m_mutex };

        if( !m_factories.count( item_type ) )
        {
            m_factories.emplace( item_type, std::move( factory ) );
            return true;
        }

        return false;
    }

    template< class GitItemType, typename = std::enable_if_t< item::is_git_item< GitItemType >::value > >
    std::unique_ptr< GitItemType > create( const item::type itemType, typename GitItemType::_internalType* manage = nullptr )
    {
        std::lock_guard< std::mutex > l{ m_mutex };

        auto factory = m_factories.find( itemType );
        if ( factory != m_factories.end() )
        {
            auto result = details::dynamic_unique_pointer_cast< GitItemType >( factory->second->create() );
            result->reset( manage );
            return result;
        }

        return nullptr;
    }

    static git_item_creator& get()
    {
        static git_item_creator instance;
        return instance;
    }

private:
    factory_storage m_factories;
    std::mutex m_mutex;
};

}// factory

}//git_handler

#endif
