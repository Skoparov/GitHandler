#ifndef GITITEMFACTORY_H
#define GITITEMFACTORY_H

#include "Deleters.h"

#include "memory"
#include "map"

class Item;
typedef std::shared_ptr<Item> ItemPtr;

//////////////////////////////////////////////////////////////////////////////
///////////////				     GitItem				//////////////////////
//////////////////////////////////////////////////////////////////////////////

class Item
{
public: 
	virtual ~Item() {}; 
};

template<class GitItemType, class Deleter>
class GitItem : public Item
{
	typedef std::shared_ptr<GitItemType> ItemPtr;

public:
	GitItem(Deleter del) : mDeleter(del) {}

	void initItem(){
		mItem = ItemPtr(new GitItemType, mDeleter);
	}

	ItemPtr item() {
		return mItem;
	};

	GitItemType* gitItem(){
		return mItem.get();
	}	

	void setItem(GitItemType* item){		
		mItem.reset(item, mDeleter);	
	}
	
	bool isValid(){
		return mItem != nullptr; 
	}	

private:
	std::shared_ptr<GitItemType> mItem;
	Deleter mDeleter;
};


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
	typedef std::shared_ptr<GitItemFactoryInterface> FactoryPtr;

public:
	template <class GitItemType, class Deleter>
	bool registerItemType(const int& itemType, Deleter del)
	{
		if (!mFactories.count(itemType))
		{
			mFactories.insert(std::make_pair(itemType, std::make_shared<GitItemFactory<GitItemType, Deleter>>(del)));
			return true;			
		}	

		return false;
	}	

	template<class GitItemType>
	std::shared_ptr<GitItemType> create(const int& itemType)
	{
		std::shared_ptr<GitItemType> item;

		auto factory = mFactories.find(itemType);
		if (factory != mFactories.end()){
			item = std::dynamic_pointer_cast<GitItemType>(factory->second->create());
		}		

		return item;
	}

	static GitItemCreator& get()
	{
		static GitItemCreator instance;
		return instance;
	}

private:
	std::map<int, FactoryPtr> mFactories;
};

#endif