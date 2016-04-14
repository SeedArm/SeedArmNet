#include "stdafx.h"

#include "seedArm_service.h"


#ifdef _DEBUG
#include <crtdbg.h>
#endif

//#include <boost/log/core.hpp>
//#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
//#include <boost/log/utility/setup/file.hpp>

#include <iostream>

class acceptorOne
	: public seedArm::event_receiver
{
public:
	virtual void onAccept(seedArm::connection::ptr spConn)
	{
		std::cout << "accept - " << spConn->socket().remote_endpoint().address().to_string() << " : " << spConn->socket().remote_endpoint().port() << std::endl;

		/*BOOST_LOG_TRIVIAL(boost::log::trivial::info) << spConn->socket().remote_endpoint().address().to_string();*/
		BOOST_LOG_TRIVIAL(info) << spConn->socket().remote_endpoint().address().to_string();
	}

	virtual void onClose(seedArm::connection::ptr spConn)
	{
		std::cout << "close  - " << spConn->socket().remote_endpoint().address().to_string() << " : " << spConn->socket().remote_endpoint().port() << std::endl;
	}

	virtual void onUpdate(unsigned long elapse)
	{

	}

	virtual void noHandler(seedArm::connection::ptr spConn, seedArm::message& msg)
	{
		_owner->close(spConn);
	}
};

class acceptorAnotherOne
	: public seedArm::event_receiver
{
public:
	virtual void onAccept(seedArm::connection::ptr spConn){
		std::cout << "accept Another One. " << spConn->socket().remote_endpoint().address().to_string() << " : " << spConn->socket().remote_endpoint().port() << std::endl;
	}

	virtual void onClose(seedArm::connection::ptr spConn){
		std::cout << "close Another One. " << spConn->socket().remote_endpoint().address().to_string() << " : " << spConn->socket().remote_endpoint().port() << std::endl;
	}

	virtual void onUpdate(unsigned long elapse){}

	virtual void noHandler(seedArm::connection::ptr spConn, seedArm::message& msg){
		_owner->close(spConn);
	}
};

class SomeManagerClass
	: public seedArm::component
{
public:
	SomeManagerClass() = default;
	virtual ~SomeManagerClass() = default;

	virtual bool initialize() override { return true; }
	virtual bool release() override { return true; }

public:
	void doSomething(){}
};

class msgHandler{
public:
	bool memberFunction(seedArm::connection::ptr impl, unsigned int size, seedArm::message& msg){
		return true;
	}
	static bool staticFunction(seedArm::connection::ptr impl, unsigned int size, seedArm::message& msg){
		return true;
	}
};
msgHandler msgHandlerInstance;

int main(void)
{
#ifdef _DEBUG
	//_CrtSetBreakAlloc(238);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	//AfxEnableMemoryTracking(allocMemDF|checkAlwaysMemDF);
	//_CrtSetBreakAlloc(5551);
#endif

	std::string acceptorAddr = "127.0.0.1";
	std::string acceptorPort = "1818";
	const size_t acceptIdx_One = 0;

	std::string acceptorAddrAnotherOne = "127.0.0.1";
	std::string acceptorPortAnotherOne = "2828";
	const size_t acceptIdx_AnotherOne = 1;


	boost::asio::io_service ios;

	acceptorOne one;
	seedArm::seedArm_service *svOne = seedArm::seedArm_service::get(acceptIdx_One, ios, one);
	{
		unsigned int protocolA = 181818;
		unsigned int protocolB = 181818;
		svOne->registMessage(protocolA, &msgHandler::memberFunction, &msgHandlerInstance);
		svOne->registMessage(protocolB, &msgHandler::staticFunction);
	}

	SomeManagerClass* someClass = svOne->addComponent<SomeManagerClass>();
	someClass->doSomething();
	svOne->getComponent<SomeManagerClass>()->doSomething();

	
	acceptorAnotherOne anotherOne;
	seedArm::seedArm_service *svAnoterOne = seedArm::seedArm_service::get(acceptIdx_AnotherOne, ios, anotherOne);
	{
		unsigned int protocolA = 181818;
		unsigned int protocolB = 181818;
		svAnoterOne->registMessage(181818, &msgHandler::memberFunction, &msgHandlerInstance);
		svAnoterOne->registMessage(282828, &msgHandler::staticFunction);
	}


	auto workerCount = boost::thread::hardware_concurrency();
	auto keepAliveTimeMs = 100000;
	svOne->acceptWait(acceptorAddr, acceptorPort, workerCount, keepAliveTimeMs);
	svAnoterOne->acceptWait(acceptorAddrAnotherOne, acceptorPortAnotherOne, 2, keepAliveTimeMs);

	svOne->run();

	delete svAnoterOne;
	delete svOne;

#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
