#pragma once

namespace seedArm
{
	template< typename T >
	class singleton
	{
	public:
		static T* instance()
		{
			if(_instance == nullptr)
			{
				_instance = new T;
			}
			return _instance;
		}

		singleton() = default;

		virtual ~singleton()
		{
			if (nullptr != _instance)
			{
				delete _instance;
				_instance = nullptr;
			}
		}

	private:
		static T* _instance;
		singleton(singleton const&) = delete;
		singleton& operator = (singleton const&) = delete;
	};

	template< typename T > T* singleton<T>::_instance = nullptr;

};//namespace seedArm
