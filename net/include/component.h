#pragma once

namespace seedArm
{
	class container;

	class component
	{
	public:
		~component() 
		{ 
			_owner = nullptr; 
		}

		virtual bool initialize() = 0;
		virtual bool release() = 0;
		virtual void onUpdate(unsigned long elapse) {}

		inline container* owner(){
			return _owner;
		}

	private:
		container* _owner;
		friend class container;
	};
};//namespace seedArm