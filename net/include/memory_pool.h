#pragma once

#include <boost/atomic.hpp>
#include <boost/pool/pool.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/shared_ptr.hpp>


//#define USE_SEED_ARM_SPINLOCK
namespace seedArm
{
#ifdef USE_SEED_ARM_SPINLOCK
	class spinLock
	{
		typedef enum{locked, unlocked} lockStatus;

	public:
		spinLock() : _status(unlocked) {}
		void lock()
		{
			while (locked == _status.exchange(locked, boost::memory_order_acquire));
		}

		void unlock()
		{
			_status.store(unlocked, boost::memory_order_release);
		}
	private:
		boost::atomic<lockStatus> _status;
	};
#endif //#ifdef USE_SEED_ARM_SPINLOCK

	template< typename __classname >
	class memory_pool
	{
	public:
		static void* operator new(size_t size) {
#ifndef USE_SEED_ARM_SPINLOCK
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			__classname* ptr = static_cast<__classname*>(_thisPool.malloc());
#else
			_lock.lock();
			__classname* ptr = static_cast<__classname*>(_thisPool.malloc());
			_lock.unlock();
#endif
			return ptr;
		}

		static void operator delete(void* ptr, size_t size) {
#ifndef USE_SEED_ARM_SPINLOCK
			boost::unique_lock<boost::shared_mutex> lock(_mutex);
			_thisPool.free(ptr);
#else
			_lock.lock();
			_thisPool.free(ptr);
			_lock.unlock();
#endif
		}
		static void destruct(__classname* ptr){
			delete ptr;
		}

	private:
		static boost::pool<> _thisPool;
#ifndef USE_SEED_ARM_SPINLOCK
		static boost::shared_mutex _mutex;
#else
		static spinLock _lock;
#endif// USE_SEED_ARM_SPINLOCK
	};

	template<typename __classname>
	boost::pool<> memory_pool<__classname>::_thisPool(sizeof(__classname));

#ifndef USE_SEED_ARM_SPINLOCK
	template<typename __classname>
	boost::shared_mutex memory_pool<__classname>::_mutex;
#else
	template<typename __classname>
	spinLock memory_pool<__classname>::_lock;
#endif// USE_SEED_ARM_SPINLOCK

};//namespace seedArm


#define DeclareMempool(classname)\
public:\
	static void* operator new(size_t size)\
{\
	classname* ptr = static_cast<classname*>(_thisPool.malloc());\
	return ptr;\
}\
	static void operator delete(void* ptr, size_t size)\
{\
	_thisPool.free(ptr);\
}\
	static void destruct(classname* ptr)\
{\
	delete ptr;\
}\
private:\
	static boost::pool<> _thisPool\

#define DeclareMempoolConstruct00(classname)\
	static boost::shared_ptr<classname> allocate_ptr()\
{\
	classname* ptr = new classname();\
	return boost::shared_ptr<classname>(ptr, classname::destruct);\
}\

#define DeclareMempoolConstruct01(classname)\
	template<typename T1>\
	static boost::shared_ptr<classname> allocate_ptr(T1 e1)\
{\
	classname* ptr = new classname(e1);\
	return boost::shared_ptr<classname>(ptr, classname::destruct);\
}\

#define DeclareMempoolConstruct02(classname)\
	template<typename T1, typename T2>\
	static boost::shared_ptr<classname> allocate_ptr(T1 e1, T2 e2)\
{\
	classname* ptr = new classname(e1, e2);\
	return boost::shared_ptr<classname>(ptr, classname::destruct);\
}\

#define DeclareMempoolConstruct03(classname)\
	template<typename T1, typename T2, typename T3>\
	static boost::shared_ptr<classname> allocate_ptr(T1 e1, T2 e2, T3 e3)\
{\
	classname* ptr = new classname(e1, e2, e3);\
	return boost::shared_ptr<classname>(ptr, classname::destruct);\
}\

#define ImplementMempool(classname)\
	boost::pool<> classname::_thisPool(sizeof(classname))\


