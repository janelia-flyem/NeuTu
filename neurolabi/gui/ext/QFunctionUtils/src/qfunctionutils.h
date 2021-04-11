#ifndef QFUNCTIONUTILS_H
#define QFUNCTIONUTILS_H

#include <functional>
#include <QSharedPointer>
#include <QTimer>

namespace QFunctionUtils
{

// function used to cache user arguments to be able to use them once timer times out
template<typename R, class ...Args>
void debounce_args_cache(const std::function<R(Args(...args))> &funcFinish, Args(&...args))
{
	funcFinish(args...);
}

typedef QSharedPointer<QMetaObject::Connection> QMetaConnPtr;
// debounce implementation
// gets a user defined function as an argument and a 'delay' in miliseconds
// returns a new function that the user can call many times, but will only execute the last
// call after 'delay' miliseconds have passed
template<typename R, class ...Args>
std::function<R(Args(...args))> debounce_internal(const std::function<R(Args(...args))> &callback, const int &delay)
{
	QTimer * timer = new QTimer;
	QMetaConnPtr conn = QMetaConnPtr(new QMetaObject::Connection,
		[timer](QMetaObject::Connection * pConn) mutable {
		// connection will be deleted when returned function goes out of scope, but not the timer
		// so we need to delete it manually
		delete pConn;
		delete timer;
	});
	timer->setInterval(delay);
	// return function that the user can call
	return [callback, timer, conn](Args(...args)) {
		// disconnect old timer callback, stop timer
		QObject::disconnect(*conn);
		timer->stop();
		// create function that caches user arguments
		std::function<void(std::function<R(Args(...args))>)> funcCache = std::bind(debounce_args_cache<R, Args...>, std::placeholders::_1, args...);
		// connect new timer callback, restart timer
		*conn = QObject::connect(timer, &QTimer::timeout,
			[callback, timer, funcCache]() {
			// stop timer
			timer->stop();
			// call after delay
			funcCache(callback);
		});
		timer->start();
		// return default constructed return type (just to support any return type)
		return R();
	};
}

// throttle implementation
// same as debounce, but also calls the given function the first time is called
template<typename R, class ...Args>
std::function<R(Args(...args))> throttle_internal(const std::function<R(Args(...args))> &callback, const int &delay)
{
	QTimer * timer = new QTimer;
	QMetaConnPtr conn = QMetaConnPtr(new QMetaObject::Connection,
		[timer](QMetaObject::Connection * pConn) mutable {
		// same logic as in debounce_internal
		delete pConn;
		delete timer;
	});
	timer->setInterval(delay);
	// return function that the user can call
	return [callback, timer, conn](Args(...args)) {
		// disconnect old timer callback
		QObject::disconnect(*conn);
		// check timer (throttle) state
		if (!timer->isActive())
		{
			// connect new timer callback
			*conn = QObject::connect(timer, &QTimer::timeout,
				[timer]() {
				// stop timer only (this happens if user did not call the function during the delay)
				timer->stop();
			});
			// start timer to start throttle
			timer->start();
			// call inmediatly and return
			return callback(args...);
		}
		// create function that caches user arguments
		std::function<void(std::function<R(Args(...args))>)> funcCache = std::bind(debounce_args_cache<R, Args...>, std::placeholders::_1, args...);
		// connect new timer callback
		*conn = QObject::connect(timer, &QTimer::timeout,
			[callback, timer, funcCache]() {
			// stop timer
			timer->stop();
			// call after delay
			funcCache(callback);
		});
		// return default constructed return type (just to support any return type)
		return R();
	};
}

// for any class implementing operator() (e.g. lambdas)
template<typename T> struct functor_traits : functor_traits<decltype(&T::operator())>
{ };
// specialization - lambda
template<typename C, typename R, typename... Args>
struct functor_traits<R(C::*)(Args...) const>
{
	using lambda_type = std::function<R(Args...)>;
};
// specialization - lambda mutable
template<typename C, typename R, typename... Args>
struct functor_traits<R(C::*)(Args...)>
{
	using lambda_type = std::function<R(Args...)>;
};
// specialization - function pointer
template<class R, class... Args>
struct functor_traits<R(*)(Args...)>
{
	using lambda_type = std::function<R(Args...)>;
};
// wrap any of the specializations above into an std::function
template<typename T> 
auto make_std_function(const T& fn) -> typename functor_traits<T>::lambda_type
{
	return typename functor_traits<T>::lambda_type(fn);
}
// convert callback into std::function and call internal debounce
template<typename T>
auto Debounce(T callback, const int &delay) -> typename functor_traits<T>::lambda_type
{
	return debounce_internal(make_std_function(callback), delay);
}
// convert callback into std::function and call internal throttle
template<typename T>
auto Throttle(T callback, const int &delay) -> typename functor_traits<T>::lambda_type
{
	return throttle_internal(make_std_function(callback), delay);
}

}

#endif // QFUNCTIONUTILS_H