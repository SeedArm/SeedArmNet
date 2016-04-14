#pragma once

#include <boost/asio.hpp>
#include <boost/chrono.hpp>
#include <string.h>

#include "seedArm_service.h"
#include "container.h"
#include "singleton.h"

namespace seedArm
{
	class IEventReceiver;

	class seedArm_service_impl
		: public seedArm_service
	{
	public:
		virtual void close(connection::ptr spConn) override;
		virtual void run() override;
		virtual boost::asio::io_service& ios() override;
		virtual boost::asio::strand& strand() override;

		inline event_receiver& receiver() { return _receiver;  }
		void initLog();

	private:
		seedArm_service_impl(boost::asio::io_service& ios, event_receiver& receiver);
		virtual ~seedArm_service_impl();
		
		void acceptWait(const std::string& addr, const std::string& port
			, size_t workerThreadsCount
			, const size_t keepAliveMilliseconds_GlobalValue
			) override;

		void accept(const boost::system::error_code& ec);
		void update(const boost::system::error_code& ec);
		void closeSocket(connection::ptr spConn);

		void msgEnque(connection::ptr conn, message::ptr msg);


		friend class connection_impl;

		connection::ptr _acceptConn;

		boost::asio::ip::tcp::acceptor	_acceptor;
		boost::asio::deadline_timer		_updater;
		boost::asio::strand				_strand;
		boost::asio::io_service::work	_work;
		boost::asio::io_service&		_ios;

		boost::chrono::system_clock::time_point _prevTime;

		friend class seedArm_service;
	};
};//namespace seedArm
