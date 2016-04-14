#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "container.h"
#include "event_receiver.h"
#include "message_procedure.h"

namespace seedArm
{
	class message;
	class connection;

	class seedArm_service :
		public container,
		public message_procedure
	{
	public:
		virtual ~seedArm_service() {}
		virtual void close(connection::ptr spConn) = 0;

		virtual void run() = 0;
		virtual void acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount
			, const size_t keepAliveMilliseconds) = 0;

		virtual boost::asio::io_service& ios() = 0;
		virtual boost::asio::strand& strand() = 0;

		static seedArm_service* get(const size_t acceptorIdx, boost::asio::io_service& ios, event_receiver& receiver);
		static seedArm_service* instance(const size_t acceptorIdx);

	protected:
		seedArm_service(event_receiver& receiver)
			: message_procedure(receiver)
		{
			receiver._owner = this;
		}
	};
};//namespace seedArm