#include <assert.h>
#include <type_traits>
#include <utility>

#include "message.h"


namespace seedArm
{
	message::message()
		: _read(0)
		, _write(0)
	{
		static_assert(
			std::is_same< decltype(msgHeader::size), unsigned int>::value
			, "msgHeader size's type must be unsigned int"
		);

		clear();
	}

	message::message(const unsigned int protocol)
	{
		clear();

		msgHeader header;
		header.size = sizeof(msgHeader);
		header.protocol = protocol;
		write((char*)&header, header.size);
	}

	message::message(const message& e)
		: _read(0)
		, _write(0)
	{
		clear();

		write(const_cast<message&>(e)._buffer, const_cast<message&>(e).size());
	}

	message::~message()
	{
		clear();
	}

	void message::clear()
	{
		_read = _buffer;
		_write = _buffer;
	}

	size_t message::size() const
	{
		return _write - _read;
	}

	void message::facilitate()
	{
		decltype(msgHeader::size) s = size() - sizeof(msgHeader);
		memcpy(_read, &s, sizeof(s));
	}

	char* message::data() const
	{
		return _read;
	}

	bool message::readable(char* dst, size_t count)
	{
		if(false == readable(count))
		{
			return false;
		}
		memcpy(dst, _read, count);
		return true;
	}

	bool message::readable(size_t count) const
	{
		return _read + count <= _write;
	}

	char* message::enableBuffer() const
	{
		return _write;
	}

	size_t message::remainWriteSize() const
	{
		return _buffer + MAX_BUFFER_SIZE - _write;
	}

	void message::arrange()
	{
		if (_read == _write){
			clear();
		}
		else
		{
			size_t count = size();
			memmove(_buffer, _read, count);
			_read = _buffer;
			_write = _read + count;
		}
	}

	message& message::write(char* src, size_t count)
	{
		if (MAX_BUFFER_SIZE < (size() + count))
		{
			assert(false && "buffer overrun in seedArm message");
		}

		memcpy(_write, src, count);
		_write += count;
		return *this;
	}


	message& message::write(size_t count)
	{
		if (MAX_BUFFER_SIZE < (size() + count))
		{
			assert(false && "buffer overrun in seedArm message");
		}

		_write += count;
		return *this;
	}

	message& message::read(char* dst, size_t count)
	{
		if(false == readable(count))
		{
			assert(false && "buffer can't readable in seedArm message");
		}

		if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
		{
			arrange();
		}

		memcpy(dst, _read, count);
		_read += count;

		return *this;
	}

	message& message::read(size_t count)
	{
		if (false == readable(count))
		{
			assert(false && "buffer can't readable in seedArm message");
		}

		if (remainWriteSize() < MAX_BUFFER_SIZE / 3 * 2)
		{
			arrange();
		}

		_read += count;

		return *this;
	}

	message& message::readAll()
	{
		_read = _write = _buffer;
		return *this;
	}
};//namespace seedArm

