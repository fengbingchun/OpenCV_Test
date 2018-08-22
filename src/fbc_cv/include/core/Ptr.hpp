// fbc_cv is free software and uses the same licence as OpenCV
// Email: fengbingchun@163.com

#ifndef FBC_CV_CORE_PTR_HPP_
#define FBC_CV_CORE_PTR_HPP_

/* reference: include/opencv2/core/cvstd.hpp
              modules/core/include/opencv2/core/ptr.inl.hpp
*/

#ifndef __cplusplus
	#error Ptr.hpp header must be compiled as C++
#endif

#ifdef _MSC_VER
	#include <intrin.h>
#endif
#include <algorithm>

namespace fbc {
#ifdef _MSC_VER
	#define CV_XADD(addr, delta) (int)_InterlockedExchangeAdd((long volatile*)addr, delta)
#else
	#define CV_XADD(addr, delte) (int)__sync_fetch_and_add((unsigned*)(addr), (unsigned)(delte))
#endif

template<typename Y>
struct DefaultDeleter {
	void operator () (Y* p) const { delete p; }
};

namespace detail {
// Metafunction to avoid taking a reference to void.
template<typename T>
struct RefOrVoid { typedef T& type; };

template<>
struct RefOrVoid<void>{ typedef void type; };

template<>
struct RefOrVoid<const void>{ typedef const void type; };

template<>
struct RefOrVoid<volatile void>{ typedef volatile void type; };

template<>
struct RefOrVoid<const volatile void>{ typedef const volatile void type; };

struct PtrOwner {
	PtrOwner() : refCount(1) {}

	void incRef()
	{
		CV_XADD(&refCount, 1);
	}

	void decRef()
	{
		if (CV_XADD(&refCount, -1) == 1) deleteSelf();
	}

protected:
	// This doesn't really need to be virtual, since PtrOwner is never deleted directly,
	// but it doesn't hurt and it helps avoid warnings.
	virtual ~PtrOwner() {}

	virtual void deleteSelf() = 0;

private:
	unsigned int refCount;

	// noncopyable
	PtrOwner(const PtrOwner&);
	PtrOwner& operator = (const PtrOwner&);
};

template<typename Y, typename D>
struct PtrOwnerImpl : PtrOwner
{
	PtrOwnerImpl(Y* p, D d) : owned(p), deleter(d) {}

	void deleteSelf()
	{
		deleter(owned);
		delete this;
	}

private:
	Y* owned;
	D deleter;
};

} // namespace detail

// Template class for smart pointers with shared ownership
// A Ptr<T> pretends to be a pointer to an object of type T. Unlike an ordinary pointer, however,
// the object will be automatically cleaned up once all Ptr instances pointing to it are destroyed.
// Ptr is similar to boost::shared_ptr that is part of the Boost library
template<typename T>
struct Ptr {
	// Generic programming support.
	typedef T element_type;

	// The default constructor creates a null Ptr - one that owns and stores a null pointer.
	Ptr();

	// If p is null, these are equivalent to the default constructor.
	// Otherwise, these constructors assume ownership of p - that is, the created Ptr owns and stores p
	// and assumes it is the sole owner of it. Don't use them if p is already owned by another Ptr
	// or else p will get deleted twice.
	// note: It is often easier to use makePtr instead.
	template<typename Y>
	explicit Ptr(Y* p);

	// overload
	// d: Deleter to use for the owned pointer, p: Pointer to own.
	template<typename Y, typename D>
	Ptr(Y* p, D d);

	// These constructors create a Ptr that shares ownership with another Ptr
	Ptr(const Ptr& o);

	// overload
	template<typename Y>
	Ptr(const Ptr<Y>& o);

	// overload
	template<typename Y>
	Ptr(const Ptr<Y>& o, T* p);

	// The destructor is equivalent to calling Ptr::release.
	~Ptr();

	// Assignment replaces the current Ptr instance with one that owns and stores same pointers as o and
	// then destroys the old instance.
	Ptr& operator = (const Ptr& o);

	// overload
	template<typename Y>
	Ptr& operator = (const Ptr<Y>& o);

	// If no other Ptr instance owns the owned pointer, deletes it with the associated deleter. Then sets
	// both the owned and the stored pointers to NULL.
	void release();

	// `ptr.reset(...)` is equivalent to `ptr = Ptr<T>(...)`.
	template<typename Y>
	void reset(Y* p);

	// overload
	// d Deleter to use for the owned pointer.
	template<typename Y, typename D>
	void reset(Y* p, D d);

	// Swaps the owned and stored pointers (and deleters, if any) of this and o.
	void swap(Ptr& o);

	// Returns the stored pointer.
	T* get() const;

	// Ordinary pointer emulation.
	typename detail::RefOrVoid<T>::type operator * () const;

	// Ordinary pointer emulation.
	T* operator -> () const;

	// Equivalent to get().
	operator T* () const;

	// ptr.empty() is equivalent to `!ptr.get()`.
	bool empty() const;

	// Returns a Ptr that owns the same pointer as this,
	// and stores the same pointer as this, except converted via static_cast to Y*.
	template<typename Y>
	Ptr<Y> staticCast() const;

	// Ditto for const_cast.
	template<typename Y>
	Ptr<Y> constCast() const;

	// Ditto for dynamic_cast.
	template<typename Y>
	Ptr<Y> dynamicCast() const;

	Ptr(Ptr&& o);
	Ptr& operator = (Ptr&& o);

private:
	detail::PtrOwner* owner;
	T* stored;

	template<typename Y>
	friend struct Ptr; // have to do this for the cross-type copy constructor
};

/////////////////////////////// Ptr impl //////////////////////////////
template<typename T>
Ptr<T>::Ptr() : owner(NULL), stored(NULL)
{}

template<typename T>
template<typename Y>
Ptr<T>::Ptr(Y* p) : owner(p ? new detail::PtrOwnerImpl<Y, DefaultDeleter<Y> >(p, DefaultDeleter<Y>()) : NULL), stored(p)
{}

template<typename T>
template<typename Y, typename D>
Ptr<T>::Ptr(Y* p, D d) : owner(p ? new detail::PtrOwnerImpl<Y, D>(p, d) : NULL), stored(p)
{}

template<typename T>
Ptr<T>::Ptr(const Ptr& o) : owner(o.owner), stored(o.stored)
{
	if (owner) owner->incRef();
}

template<typename T>
template<typename Y>
Ptr<T>::Ptr(const Ptr<Y>& o) : owner(o.owner), stored(o.stored)
{
	if (owner) owner->incRef();
}

template<typename T>
template<typename Y>
Ptr<T>::Ptr(const Ptr<Y>& o, T* p) : owner(o.owner), stored(p)
{
	if (owner) owner->incRef();
}

template<typename T>
Ptr<T>::~Ptr()
{
	release();
}

template<typename T>
Ptr<T>& Ptr<T>::operator = (const Ptr<T>& o)
{
	Ptr(o).swap(*this);
	return *this;
}

template<typename T>
template<typename Y>
Ptr<T>& Ptr<T>::operator = (const Ptr<Y>& o)
{
	Ptr(o).swap(*this);
	return *this;
}

template<typename T>
void Ptr<T>::release()
{
	if (owner) owner->decRef();
	owner = NULL;
	stored = NULL;
}

template<typename T>
template<typename Y>
void Ptr<T>::reset(Y* p)
{
	Ptr(p).swap(*this);
}

template<typename T>
template<typename Y, typename D>
void Ptr<T>::reset(Y* p, D d)
{
	Ptr(p, d).swap(*this);
}

template<typename T>
void Ptr<T>::swap(Ptr<T>& o)
{
	std::swap(owner, o.owner);
	std::swap(stored, o.stored);
}

template<typename T>
T* Ptr<T>::get() const
{
	return stored;
}

template<typename T>
typename detail::RefOrVoid<T>::type Ptr<T>::operator * () const
{
	return *stored;
}

template<typename T>
T* Ptr<T>::operator -> () const
{
	return stored;
}

template<typename T>
Ptr<T>::operator T* () const
{
	return stored;
}

template<typename T>
bool Ptr<T>::empty() const
{
	return !stored;
}

template<typename T>
template<typename Y>
Ptr<Y> Ptr<T>::staticCast() const
{
	return Ptr<Y>(*this, static_cast<Y*>(stored));
}

template<typename T>
template<typename Y>
Ptr<Y> Ptr<T>::constCast() const
{
	return Ptr<Y>(*this, const_cast<Y*>(stored));
}

template<typename T>
template<typename Y>
Ptr<Y> Ptr<T>::dynamicCast() const
{
	return Ptr<Y>(*this, dynamic_cast<Y*>(stored));
}

template<typename T>
Ptr<T>::Ptr(Ptr&& o) : owner(o.owner), stored(o.stored)
{
	o.owner = NULL;
	o.stored = NULL;
}

template<typename T>
Ptr<T>& Ptr<T>::operator = (Ptr<T>&& o)
{
	release();
	owner = o.owner;
	stored = o.stored;
	o.owner = NULL;
	o.stored = NULL;
	return *this;
}

/////////////////////////////////////////////////////
template<typename T>
Ptr<T> makePtr()
{
	return Ptr<T>(new T());
}

template<typename T, typename A1>
Ptr<T> makePtr(const A1& a1)
{
	return Ptr<T>(new T(a1));
}

template<typename T, typename A1, typename A2>
Ptr<T> makePtr(const A1& a1, const A2& a2)
{
	return Ptr<T>(new T(a1, a2));
}

template<typename T, typename A1, typename A2, typename A3>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3)
{
	return Ptr<T>(new T(a1, a2, a3));
}

template<typename T, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
Ptr<T> makePtr(const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
	return Ptr<T>(new T(a1, a2, a3, a4, a5, a6));
}

} // namespace fbc

#endif // FBC_CV_CORE_PTR_HPP_
