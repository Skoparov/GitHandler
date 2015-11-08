#ifndef GITITEMFACTORY_H
#define GITITEMFACTORY_H

#include "map"
#include "GitItem.h"

//////////////////////////////////////////////////////////////////////////////
///////////////           GitItemFactoryInterface       //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitItemFactoryInterface
{	
public:
	virtual ~GitItemFactoryInterface(){};
	virtual ItemPtr create() = 0;	
};

//////////////////////////////////////////////////////////////////////////////
///////////////               GitItemFactory            //////////////////////
//////////////////////////////////////////////////////////////////////////////

template <class GitItemType, class Deleter>
class GitItemFactory : public GitItemFactoryInterface
{
	typedef GitItem<GitItemType, Deleter> CurrGitItem;

public:
	GitItemFactory(Deleter del) : mDeleter(del) {}

	ItemPtr create(){
		return std::make_shared<CurrGitItem>(mDeleter);
	}	

private:
	Deleter mDeleter;
};

//////////////////////////////////////////////////////////////////////////////
///////////////                GitItemCreator           //////////////////////
//////////////////////////////////////////////////////////////////////////////

class GitItemCreator
{
	typedef std::map<int, std::shared_ptr<GitItemFactoryInterface>> FactoryStorage;

public:
	template <class GitItemType, class Deleter>
	static bool registerItemType(const int& itemType, Deleter del)
	{
		if (!mFactories.count(itemType))
		{
			auto factoryData = std::make_pair(itemType, std::make_shared<GitItemFactory<GitItemType, Deleter>>(del));
			mFactories.insert(factoryData);
			return true;			
		}	

		return false;
	}	

	template<class GitItemType>
	static std::shared_ptr<GitItemType> create(const int& itemType)
	{
		std::shared_ptr<GitItemType> item;

		auto factory = mFactories.find(itemType);
		if (factory != mFactories.end()){
			item = std::dynamic_pointer_cast<GitItemType>(factory->second->create());
		}		

		return item;
	}	

private:
	static FactoryStorage mFactories;
};

__declspec(selectany) GitItemCreator::FactoryStorage GitItemCreator::mFactories;

#endif