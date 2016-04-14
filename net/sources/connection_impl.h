#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "connection.h"
#include "seedArm_service.h"
#include "memory_pool.h"
#include "message.h"

namespace seedArm
{
	class connection_impl
		: public connection
		, public boost::enable_shared_from_this< connection_impl >
		, public memory_pool< connection_impl >
	{
	public:
		connection_impl(seedArm_service* owner, boost::asio::io_service& ios);
		virtual ~connection_impl();

		virtual void set_status(const status s);
		virtual status get_status() const;
		virtual boost::asio::ip::tcp::socket& socket();
		virtual boost::asio::strand& strand();
		virtual seedArm_service* owner();

		virtual void start();
		virtual void send(message& msg);

		virtual void  setTag(void* tag);
		virtual void* getTag() const;

	private:
		void read_handler	(const boost::system::error_code& ec, size_t bytes_transferred);
		void write_handler	(const boost::system::error_code& ec, size_t bytes_transferred);

		void renewExpireTime();
		void onExpireTime(const boost::system::error_code& ec);

		status _status;
		boost::asio::strand _strand;
		boost::asio::ip::tcp::socket _socket;
		seedArm_service* _owner;
		boost::asio::deadline_timer* _keepTimer;
		message::ptr _msg;
		void* _tag;
	};
};//namespace seedArm
