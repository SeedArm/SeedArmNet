#pragma once

#include <boost/unordered_map.hpp>
#include "component.h"

namespace seedArm
{
	class container
	{
	public:
		~container()
		{
			for (auto it : _components ){
				delete it.second;
			}
			_components.clear();
		}

		template< typename __component > __component* addComponent();
		template< typename __component > __component* getComponent();
		template< typename __component > bool removeComponent();

	protected:
		boost::unordered_map< size_t, component* > _components;
	};

	template< typename __component > 
	__component* container::addComponent()
	{
		__component* component = nullptr;

		size_t typeCode = typeid(__component).hash_code();
		auto it = _components.find(typeCode);

		if (_components.end() == it)
		{
			component = new __component;
			component->_owner = this;
		}

		if(false == component->initialize()) 
		{
			delete component;
			return nullptr;
		}

		_components.insert(std::make_pair(typeCode, component));
		return component;
	}

	template< typename __component >
	__component* container::getComponent() 
	{
		size_t typeCode = typeid(__component).hash_code();
		auto it = _components.find(typeCode);
		if (_components.end() == it){
			return nullptr;
		}
		return static_cast<__component*>(it->second);
	}

	template< typename __component >
	bool container::removeComponent() 
	{
		size_t typeCode = typeid(__component).hash_code();
		auto it = _components.find(typeCode);
		if (_components.end() == it){
			return false;
		}

		it->second->release();
		delete it->second;
		_components.erase(it);

		return true;
	}
};//namespace seedArm

