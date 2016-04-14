#pragma once

#include "connection.h"

namespace seedArm
{
	class seedArm_service;

	class event_receiver
	{
	public:
		virtual void onAccept(connection::ptr spConn) = 0;
		virtual void onClose(connection::ptr spConn) = 0;
		virtual void onUpdate(unsigned long elapse) = 0;
		virtual void noHandler(connection::ptr spConn, message& msg) = 0;

	protected:
		seedArm_service* _owner;
		friend class seedArm_service;
	};
};//namespace seedArm
