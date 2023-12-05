#pragma once

#include <stdexcept>

template<class T>
class SmartPtr {
public:
	SmartPtr(T* typ);
	SmartPtr(T typ);
	~SmartPtr();

	SmartPtr(const SmartPtr& from);
	SmartPtr(SmartPtr&& from);
	

	// attempting to copy when disallowed will throw
	void SetAllowCopy(bool b);

	// remove the pointer this container is holding (deleting if this is the last holder)
	// and, optionally, replace it with a new pointer
	void Reset();
	void Reset(T* typ);

	// alias of Reset() with no args supplied
	void Release();

	// return the pointer this container is holding
	T* Get();


private:
	T* ptr_;
	size_t* refPtr_;
	bool allowCopy_ = true;

	void decrRef();
	void incrRef();
	bool hasRefs();
	bool refCheck();
};

// These have to be in the header file, i have no clue why; linker complains!!!!!!!

// internals
template<class T>
void SmartPtr<T>::incrRef() {
	(*refPtr_)++;
}

template<class T>
void SmartPtr<T>::decrRef() {
	(*refPtr_)--;
}

template<class T>
bool SmartPtr<T>::hasRefs() {
	return (*refPtr_) != 0;
}

template <class T>
bool SmartPtr<T>::refCheck() {
	if (!hasRefs()) {
		delete ptr_;
		delete refPtr_;
		return false;
	}

	return true;
}


// ctors & dtors
template<class T>
SmartPtr<T>::SmartPtr(T* typ) : ptr_{ typ } {
	refPtr_ = new size_t;
	*refPtr_ = 1;
	allowCopy_ = true;
}

template<class T>
SmartPtr<T>::SmartPtr(T typ) : ptr_{ &typ } {
	refPtr_ = new size_t;
	*refPtr_ = 1;
	allowCopy_ = true;
}

template<class T>
SmartPtr<T>::~SmartPtr() {
	if (ptr_ != nullptr) {
		decrRef();
		refCheck();
	}
}

// copy: copy the pointers and add 1 to refcount
template<class T>
SmartPtr<T>::SmartPtr(const SmartPtr& from) {
	if (!from.allowCopy_) {
		throw std::out_of_range("Copying disallowed");
	}

	refPtr_ = from.refPtr_;
	ptr_ = from.ptr_;
	incrRef();
}

// move: move the pointers; don't add 1 since old ptr becomes invalid
template<class T>
SmartPtr<T>::SmartPtr(SmartPtr&& from) {
	ptr_ = from.ptr_;
	from.ptr_ = nullptr;
}

template <class T>
void SmartPtr<T>::SetAllowCopy(bool b) { allowCopy_ = b; }

template<class T>
T* SmartPtr<T>::Get() {
	return ptr_;
}

template<class T>
void SmartPtr<T>::Reset() {
	decrRef();

	if (!hasRefs()) {
		refCheck();
	}

	ptr_ = nullptr;
}

template<class T>
void SmartPtr<T>::Reset(T* newPtr) {
	Reset();
	ptr_ = newPtr;
}