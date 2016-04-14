#pragma once

#include <boost/unordered_map.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "connection.h"
#include "message.h"

namespace seedArm
{
	class event_receiver;

	class message_procedure
	{
	public:
		using msgHandlerFn = boost::function<bool(connection::ptr, unsigned int, message&)>;

		message_procedure(event_receiver& receiver);
		~message_procedure();

		bool registMessage(unsigned int protocol, msgHandlerFn fn);

		template< typename __type >
		bool registMessage(
			unsigned int protocol
			, bool(__type::*fn)(connection::ptr, unsigned int, message&)
			, __type* instance) 
		{
			auto it = _instanceElements.find(protocol);

			if (_instanceElements.end() != it) {
				return false;
			}

			_instanceElements.insert(std::make_pair(protocol, boost::bind(fn, instance, _1, _2, _3)));
			return true;
		}

	protected:
		bool dispatch(connection::ptr spConn, message::ptr msg);

	private:
		boost::unordered_map<unsigned int, msgHandlerFn > _instanceElements;
		boost::unordered_map<unsigned int, msgHandlerFn> _staticElements;

	protected:
		event_receiver& _receiver;
	};
};//namespace seedArm
