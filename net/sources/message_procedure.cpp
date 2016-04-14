#include "message_procedure.h"

#include "event_receiver.h"

namespace seedArm
{
	message_procedure::message_procedure(event_receiver& receiver)
		: _receiver(receiver)
	{
	}

	message_procedure::~message_procedure()
	{
		_instanceElements.clear();
		_staticElements.clear();
	}

	bool message_procedure::registMessage(unsigned int protocol, msgHandlerFn fn) {
		auto it = _staticElements.find(protocol);

		if (_staticElements.end() != it) {
			return false;
		}

		_staticElements.insert(std::make_pair(protocol, fn));
		return true;
	}

	bool message_procedure::dispatch(connection::ptr spConn, message::ptr msg)
	{
		do 
		{
			message::msgHeader header;
			int sizeHeader = sizeof(header);

			if (false == msg->readable((char*)&header, sizeHeader)) break;
			if (false == msg->readable(header.size)) break;

			msg->read(sizeHeader);

			auto it = _instanceElements.find(header.protocol);
			if(_instanceElements.end() != it) {
				it->second(spConn, header.size, *msg.get());
			}
			else{
				auto it = _staticElements.find(header.protocol);
				if(_staticElements.end() != it){
					it->second(spConn, header.size, *msg.get());
				}
				else{
					_receiver.noHandler(spConn, *msg.get());
					msg->readAll();
					break;
				}
			}
		}while(true);

		return true;
	}
};//namespace seedArm
