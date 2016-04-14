#pragma once

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

namespace seedArm
{
	class message;
	class seedArm_service;

	class connection
	{
	public:
		using ptr = boost::shared_ptr<connection>;

		enum class status : unsigned char {
			open = 0,
			closing = 1,
			close = 2,
		};

		static size_t KEEP_ALIVE_TIME_MS;

	public:
		virtual ~connection();

		virtual void set_status(const status s) = 0;
		virtual status get_status() const = 0;
		virtual boost::asio::ip::tcp::socket& socket() = 0;
		virtual boost::asio::strand& strand() = 0;
		virtual seedArm_service* owner() = 0;

		virtual void start() = 0;
		virtual void send(message& msg) = 0;

		virtual void setTag(void* tag) = 0;
		virtual void* getTag() const = 0;
	};
};//namespace seedArm

