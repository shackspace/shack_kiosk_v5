#ifndef PROTECTED_VALUE_HPP
#define PROTECTED_VALUE_HPP

#include <mutex>

template<typename T>
struct protected_value;

//!
//! Value accessor of a protected_value<T>.
//! Allows access via *, -> and value() method.
//!
template<typename T>
struct value_access
{
	typedef std::remove_const_t<T> underlying_type;
	friend struct protected_value<underlying_type>;
private:
	protected_value<underlying_type> & _value;
	std::lock_guard<std::mutex> _guard;

	value_access(protected_value<underlying_type> & val) :
		_value(val),
	  _guard(val.mutex)
	{

	}

public:
	T & value();
	T const & value() const;

public:
	T &       operator*()       { return value(); }
	T const & operator*() const { return value(); }

	T *       operator->()       { return &value(); }
	T const * operator->() const { return &value(); }

	T &       operator= (T const & other) { return value() = other; }
	T &       operator= (T && other)      { return value() = std::move(other); }
};

//!
//! A value guarded by a mutex.
//! Must call obtain() to receive access to the handle.
//!
template<typename T>
struct protected_value
{
private:
	T value;
	std::mutex mutex;
public:
	friend struct value_access<T>;

	protected_value(T const & value = T { }) :
	  value(value)
	{
	}

	protected_value(T && value) :
	  value(std::move(value))
	{
	}

	value_access<T>       obtain()       { return value_access<T> { *this }; }
	value_access<const T> obtain() const { return value_access<const T> { *this }; }
};


template<typename T>
T & value_access<T>::value() {
	return _value.value;
}

template<typename T>
T const & value_access<T>::value() const {
	return _value.value;
}

#endif // PROTECTED_VALUE_HPP
