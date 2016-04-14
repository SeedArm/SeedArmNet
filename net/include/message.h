#pragma once

#include <boost/shared_ptr.hpp>
#include <vector>

#include "memory_pool.h"

namespace seedArm
{
	class message
		: public memory_pool< message >
	{
	public:
		using ptr = boost::shared_ptr< message >;

		static const size_t	MAX_BUFFER_SIZE = 1024 * 8;

		struct msgHeader{
			unsigned int size;
			unsigned int protocol;
		};

		message();
		message(const unsigned int protocol);
		message(const message& e);
		virtual ~message();

		void clear();
		size_t size() const;
		void facilitate();
		char* data() const;
		bool readable(char* dst, size_t count);
		bool readable(size_t count) const;
		char* enableBuffer() const;
		size_t remainWriteSize() const;
		void arrange();

		message& write(char* src, size_t count);
		message& write(size_t count);

		message& read(char* dst, size_t count);
		message& read(size_t count);
		message& readAll();

		template< typename T >
		inline message& operator >> (T& b) {
			return read((char*)&b, sizeof(b));
		}
		template< typename T >
		inline message& operator >> (std::vector<T>& b) {
			size_t size;
			(*this) >> size;
			b.reserve((std::vector<T>::size_type)size);
			b.resize((std::vector<T>::size_type)0);

			for(size_v i = 0; i < size; ++i)
			{
				T e;
				(*this) >> e;
				b.push_back(std::move(e));
			}
			return *this;
		}

		inline message& operator >> (std::basic_string<char, std::char_traits<char>, std::allocator<char>>& b)
		{
			int size;
			(*this) >> size;
			if(size)
			{
				b.assign((char*)_read, size);
				_read += (size * sizeof(char));
			}

			return *this;
		}


		template< typename T >
		inline message& operator << (T& b) {
			return write((char*)&b, sizeof(b));
		}
		template< typename T >
		inline message& operator << (std::vector<T>& b) {
			size_t size = (size_t)b.size();
			(*this) << size;

			std::vector<T>::iterator it = b.begin();
			while(it != b.end())
			{
				(*this) << (*it);
				++it;
			}
			return *this;
		}

		inline message& operator << (std::basic_string<char, std::char_traits<char>, std::allocator<char>>& b)
		{
			int size = 0;
			if(b.compare(""))
			{
				size = b.length();
			}

			(*this) << size;
			if(size)
			{
				write((char*)b.c_str(), b.length() * sizeof(char));
			}
			return *this;
		}

	private:

		char	_buffer[MAX_BUFFER_SIZE];
		char*	_read;
		char*	_write;
	};
};//namespace seedArm
