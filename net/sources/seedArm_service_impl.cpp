#include "seedArm_service_impl.h"

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/core.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/attributes/named_scope.hpp>


#include <vector>


#include "connection_impl.h"
#include "event_receiver.h"

#define UPDATE_TIME_MS		54

namespace seedArm
{
	enum{
		MAX_NET_ACCEPTOR_COUNT = 10
	};

	static seedArm_service_impl* s_svr[MAX_NET_ACCEPTOR_COUNT] = {0,};
	static std::vector<boost::shared_ptr<boost::thread>> s_threads;

	seedArm_service* seedArm_service::get(const size_t acceptorIdx, boost::asio::io_service& ios, event_receiver& receiver)
	{
		assert(MAX_NET_ACCEPTOR_COUNT > acceptorIdx);

		if (nullptr == s_svr[acceptorIdx])
		{
			s_svr[acceptorIdx] = new seedArm_service_impl(ios, receiver);
		}
		return s_svr[acceptorIdx];
	}

	seedArm_service* seedArm_service::instance(const size_t acceptorIdx)
	{
		return s_svr[acceptorIdx];
	}

	seedArm_service_impl::seedArm_service_impl(boost::asio::io_service& ios, event_receiver& receiver)
		: seedArm_service(receiver)
		, _updater(ios, boost::posix_time::milliseconds(UPDATE_TIME_MS))
		, _strand(ios)
		, _acceptor(ios)
		, _work(ios)
		, _ios(ios)
	{
	}


	seedArm_service_impl::~seedArm_service_impl()
	{
	}

	void seedArm_service_impl::close(connection::ptr spConn)
	{
		spConn->set_status(connection::status::closing);

		_ios.post(
			_strand.wrap(boost::bind(
				&seedArm_service_impl::closeSocket
				, this
				, spConn)));
	}


	void seedArm_service_impl::msgEnque(connection::ptr spConn, message::ptr msg)
	{
		_ios.post(
			spConn->strand().wrap(boost::bind(
				&seedArm_service_impl::dispatch
				, this
				, spConn
				, msg)));
	}


	void seedArm_service_impl::acceptWait(const std::string& addr, const std::string& port
		, size_t workerThreadsCount
		, const size_t keepAliveMilliseconds_GlobalValue)
	{
		if (0 == workerThreadsCount){
			workerThreadsCount = boost::thread::hardware_concurrency();
		}

		////////////////////////////////////////////////
		// set common keepAliveTime
		connection_impl::KEEP_ALIVE_TIME_MS = keepAliveMilliseconds_GlobalValue;



		using TCP = boost::asio::ip::tcp;

		////////////////////////////////////////////////
		// ready accpetor
		TCP::resolver resolver(_ios);
		TCP::resolver::query quary(addr, port);
		TCP::endpoint endpoint = *resolver.resolve(quary);

		_acceptor.open(endpoint.protocol());

		_acceptor.set_option(TCP::no_delay(true));
		_acceptor.set_option(TCP::acceptor::reuse_address(true));
		_acceptor.bind(endpoint);
		_acceptor.listen();

		_acceptConn = connection::ptr(new connection_impl(this, _ios), connection_impl::destruct);
		_acceptor.async_accept(_acceptConn->socket(), _strand.wrap(
			boost::bind(&seedArm_service_impl::accept, this, boost::asio::placeholders::error)));


		////////////////////////////////////////////////
		// ready worker
		for (size_t i = 0; i < workerThreadsCount; ++i)
		{
			boost::shared_ptr<boost::thread> thread(new boost::thread(
				boost::bind(&boost::asio::io_service::run, &_ios)));

			s_threads.push_back(thread);
		}


		////////////////////////////////////////////////
		// ready expireTimer
		_prevTime = boost::chrono::system_clock::now();
		_updater.expires_from_now(boost::posix_time::milliseconds(UPDATE_TIME_MS));

		_updater.async_wait(_strand.wrap(
			boost::bind(&seedArm_service_impl::update, this, boost::asio::placeholders::error)));
	}

	void seedArm_service_impl::run()
	{
		initLog();

		BOOST_LOG_TRIVIAL(info) << "seedArmNet run..";

		_ios.run();

		for (auto it : s_threads){
			it->join();
		}
		s_threads.clear();

		BOOST_LOG_TRIVIAL(info) << "seedArmNet stop..";
	}

	boost::asio::io_service& seedArm_service_impl::ios()
	{
		return _ios;
	}

	boost::asio::strand& seedArm_service_impl::strand()
	{
		return _strand;
	}

	void seedArm_service_impl::closeSocket(connection::ptr spConn)
	{
		boost::weak_ptr<connection> wp = spConn;
		if (!wp.expired() && wp.lock() != nullptr)
		{
			try {
				boost::system::error_code ec_;
				spConn->socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec_);

				if(!ec_ && spConn->get_status() == connection::status::closing)
				{
					spConn->set_status(connection::status::close);
					_receiver.onClose(spConn);
				}
				else
				{
					std::cout << "status check : " << ec_.message().data() << std::endl;
				}
			}catch(std::exception e) {
				std::cout << "exception in seedArm_service_impl::closeSocket() => " << e.what() << std::endl;
			}
		}
		else
		{
			std::cout << "seedArm_service_impl::closeSocket(). already expired" << std::endl;
		}
	}


	void seedArm_service_impl::accept(const boost::system::error_code& ec)
	{
		if(!ec)
		{
			_receiver.onAccept(_acceptConn);

			_acceptConn->start();

			_acceptConn = connection::ptr(new connection_impl(this, _ios), connection_impl::destruct);
			_acceptor.async_accept(_acceptConn->socket(), _strand.wrap(
				boost::bind(&seedArm_service_impl::accept, this, boost::asio::placeholders::error)));
		}
	}


	void seedArm_service_impl::update(const boost::system::error_code& ec)
	{
		boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();
		unsigned long elapse = (unsigned long)boost::chrono::duration_cast<boost::chrono::milliseconds>(now - _prevTime).count();
		_prevTime = now;

		_receiver.onUpdate(elapse);

		_updater.expires_from_now(boost::posix_time::milliseconds(UPDATE_TIME_MS));
		_updater.async_wait(_strand.wrap(
			boost::bind(&seedArm_service_impl::update, this, boost::asio::placeholders::error)));
	} 

	void seedArm_service_impl::initLog()
	{
		using namespace boost::log;

		core::get()->add_global_attribute("TimeStamp", attributes::utc_clock());
		core::get()->add_global_attribute("Scope", attributes::named_scope());

		core::get()->set_filter(trivial::severity >= trivial::info);

		/* log formatter:
		* [TimeStamp] [ThreadId] [Severity Level] [Scope] Log message
		*/
		auto fmtTimeStamp = expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f");
		auto fmtThreadId = expressions::
			attr<attributes::current_thread_id::value_type>("ThreadID");
		auto fmtSeverity = expressions::
			attr<trivial::severity_level>("Severity");
		auto fmtScope = expressions::format_named_scope("Scope",
			keywords::format = "%n(%f:%l)",
			keywords::iteration = expressions::reverse,
			keywords::depth = 2);
		formatter logFmt =
			expressions::format("[%1%] (%2%) [%3%] [%4%] %5%")
			% fmtTimeStamp % fmtThreadId % fmtSeverity % fmtScope
			% expressions::smessage;

		/* console sink */
		auto consoleSink = add_console_log(std::clog);
		consoleSink->set_formatter(logFmt);

		/* fs sink */
		auto fsSink = add_file_log(
			keywords::file_name = "logs/test_%Y-%m-%d_%H-%M-%S.%N.log",
			keywords::rotation_size = 10 * 1024 * 1024, //10mb마다 rotate
			keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), //12시마다 rotate
			keywords::min_free_space = 30 * 1024 * 1024,
			keywords::auto_flush = true,
			keywords::open_mode = std::ios_base::app);
		fsSink->set_formatter(logFmt);
		/*fsSink->locked_backend()->auto_flush(true);*/
	}

};//namespace seedArm


