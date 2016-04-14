#include "connection_impl.h"

#include <boost/bind.hpp>
#include "seedArm_service_impl.h"

namespace seedArm
{
	size_t connection_impl::KEEP_ALIVE_TIME_MS;

	connection::~connection()
	{
	}

	connection_impl::connection_impl(seedArm_service* owner, boost::asio::io_service& ios)
		: _owner(owner)
		, _socket(ios)
		, _strand(ios)
		, _status(status::close)
		, _tag(nullptr)
	{
		_keepTimer = new boost::asio::deadline_timer(ios, boost::posix_time::milliseconds(KEEP_ALIVE_TIME_MS));
	}

	connection_impl::~connection_impl()
	{
		_owner = nullptr;
		_tag = nullptr;
		_status = status::close;
	}

	void connection_impl::set_status(const status s)
	{
		_status = s;
	}

	connection::status connection_impl::get_status() const
	{
		return _status;
	}

	boost::asio::ip::tcp::socket& connection_impl::socket()
	{
		return _socket;
	}

	boost::asio::strand& connection_impl::strand()
	{
		return _strand;
	}

	seedArm_service* connection_impl::owner()
	{
		return _owner;
	}

	void connection_impl::setTag(void* tag)
	{
		_tag = tag;
	}

	void* connection_impl::getTag() const
	{
		return _tag;
	}

	void connection_impl::start()
	{
		_msg = message::ptr(new message(), message::destruct);

		_socket.set_option(boost::asio::ip::tcp::no_delay(true));

		_socket.async_read_some(
			boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
			, _strand.wrap( boost::bind(&connection_impl::read_handler
										, shared_from_this()
										, boost::asio::placeholders::error
										, boost::asio::placeholders::bytes_transferred)));

		_status = status::open;

		renewExpireTime();
	}

	void connection_impl::send(message& msg)
	{
		msg.facilitate();

		_socket.async_write_some(boost::asio::buffer(msg.data(), msg.size())
			, _strand.wrap(boost::bind(&connection_impl::write_handler
										, shared_from_this()
										, boost::asio::placeholders::error
										, boost::asio::placeholders::bytes_transferred)));

	}

	void connection_impl::read_handler(const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if(!ec)
		{
			_msg->write(bytes_transferred);

			message::ptr msgRecv = _msg;
			_msg = message::ptr(new message(), message::destruct);

			((seedArm_service_impl*)_owner)->msgEnque(shared_from_this(), msgRecv);

			_socket.async_read_some(
				boost::asio::buffer(_msg->enableBuffer(), _msg->remainWriteSize())
				, _strand.wrap(boost::bind(&connection_impl::read_handler
				, shared_from_this()
				, boost::asio::placeholders::error
				, boost::asio::placeholders::bytes_transferred)));

			renewExpireTime();
		}
		else
		{
			boost::system::error_code ec_;
			_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec_);
			if(!ec_ && _status == status::open)
			{
				_status = status::closing;
				_owner->close(shared_from_this());
			}
		}
	}

	void connection_impl::write_handler(const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if(ec)
		{
			boost::system::error_code ec_;
			_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec_);
			if(!ec_ && _status == status::open)
			{
				_status = status::closing;
				_owner->close(shared_from_this());
			}

			return;
		}

		renewExpireTime();
	}


	void connection_impl::renewExpireTime()
	{
		_keepTimer->expires_from_now(
			boost::posix_time::milliseconds(KEEP_ALIVE_TIME_MS));

		_keepTimer->async_wait(_strand.wrap(
			boost::bind(&connection_impl::onExpireTime
						, this
						, boost::asio::placeholders::error)));
	}

	void connection_impl::onExpireTime(const boost::system::error_code& ec)
	{
		/*
		if(_status == status::open)
		{
			_status = status::closing;
			_owner->close(shared_from_this());
		}
		*/
	}
};//namespace seedArm
