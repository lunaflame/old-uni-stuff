#include "flatmap.h"

#include <stdio.h>
#include <assert.h>
#include <stdexcept>
#include <tuple>

// i told you grow ratio <= 1 is death!!!!!!!!
static_assert(FlatMap::DefaultCap > 0, "you fool");

// i told you grow ratio <= 1 is death!!!!!!!!
static_assert(FlatMap::GrowRatio > 1, "you absolute fool");

// ctors & dtors

typedef std::tuple<size_t, bool> FindResult;

FlatMap::FlatMap(size_t cap) : cap_(cap)  {
	this->keys_ = new Key[this->cap_];
	this->values_ = new Value[this->cap_];
}

FlatMap::FlatMap() : FlatMap(DefaultCap) {}

FlatMap::~FlatMap() {
	freeMem();
}

void FlatMap::freeMem() {
	if (keys_ != nullptr) {
		delete[] this->keys_;
		delete[] this->values_;
	}
}

// now the fun stuff

// primary
bool FlatMap::insert(const Key& key, const Value& val) {
	if (keys_ == nullptr) {
		throw std::logic_error("Tried to insert into a dead FlatMap!");
	}

	FindResult res = findInd(key);

	if (std::get<1>(res)) {
		// found an entry with the same key; just replace the value
		// is this expected behavior or is it supposed to cry and complain about an existing key???
		
		/*values_[ind] = val;
		return true;*/

		// update: i think it's supposed to return false
		return false;
	} else {
		// no entries found; insert a new one at index
		// first shift all values to the right - this sucks!!!
		// insertion time is SUPER AWFUL!!!
		
		this->incrSize();
		size_t ind = std::get<0>(res);
		for (size_t shift = sz_; shift > ind; shift--) {
			values_[shift] = values_[shift - 1];
			keys_[shift] = keys_[shift - 1];
		}

		keys_[ind] = key;
		values_[ind] = val;
		return true;
	}

}

bool FlatMap::contains(const Key& key) const {
	return std::get<1>(findInd(key));
}

const Value& FlatMap::at(const Key& k) const {
	FindResult ind = findInd(k);

	if (std::get<1>(ind)) {
		return values_[std::get<0>(ind)];
	}
	else {
		throw std::out_of_range("zomg OOR!!!!!"); // how do throws work
	}
}

Value& FlatMap::at(const Key& k) {
	FindResult ind = findInd(k);

	if (std::get<1>(ind)) {
		return values_[std::get<0>(ind)];
	}
	else {
		throw std::out_of_range("zomg OOR!!!!!"); // how do throws work
	}
}


// internals
void FlatMap::growSize(size_t newCap) {
	assert(newCap > cap_);

	Key* newKeys = new Key[newCap];
	Value* newValues = new Value[newCap];

	for (size_t i = 0; i < cap_; i++) {
		// theres probably a fancy std function for this
		newKeys[i] = keys_[i];
		newValues[i] = values_[i];
	}

	freeMem(); // delete old arrays

	this->keys_ = newKeys; // then replace them with newly-allocated ones
	this->values_ = newValues;
	this->cap_ = newCap;
}

void FlatMap::incrSize() {
	this->sz_++;
	if (this->sz_ >= this->cap_) {
		this->growSize( this->cap_ * this->GrowRatio );
	}
}

// TODO: return a tuple
FindResult FlatMap::findInd(const Key& key) const {
	if (keys_ == nullptr) {
		throw std::logic_error("Tried to find key/value in a dead FlatMap!");
	}

	size_t high, low;
	high = sz_; low = 0;

	while (low < high) {
		size_t mid = low + (high - low) / 2;

		if (key < keys_[mid]) {
			// look lower than middle
			high = mid;
		}
		else if (key == keys_[mid]) {
			// found, cool
			return std::make_tuple(mid, true);
		}
		else {
			// look higher than middle
			low = mid + 1;
		}
	}

	return std::make_tuple(low, key == keys_[low]);
}

void FlatMap::swap(FlatMap& with) {
	std::swap(with.keys_, this->keys_);
	std::swap(with.values_, this->values_);
	std::swap(with.sz_, this->sz_);
	std::swap(with.cap_, this->cap_);
}

void FlatMap::clear() {
	sz_ = 0; // lol
}

bool FlatMap::erase(const Key& key) {
	FindResult res = findInd(key);

	if (!std::get<1>(res)) {
		return false;
	}
	else {
		size_t ind = std::get<0>(res);
		for (size_t shift = ind; shift < sz_; shift++) {
			values_[shift] = values_[shift + 1];
			keys_[shift] = keys_[shift + 1];
		}

		sz_--;
		return true;
	}
}

size_t FlatMap::size() const {
	return sz_;
}

bool FlatMap::empty() const {
	return sz_ == 0;
}

// operators
void FlatMap::doCopy(const FlatMap& from, FlatMap& to) {
	to.freeMem();

	to.sz_ = from.sz_;
	to.cap_ = from.cap_;
	to.keys_ = new Key[cap_];
	to.values_ = new Value[cap_];

	for (size_t i = 0; i < sz_; i++) {
		to.keys_[i] = from.keys_[i];
		to.values_[i] = from.values_[i];
	}
}

FlatMap::FlatMap(const FlatMap& src) {
	doCopy(src, *this);
}

FlatMap::FlatMap(FlatMap&& src) {
	sz_ = src.sz_;
	cap_ = src.cap_;

	// we'll be replacing our arrays with the ones from src;
	// free ours so we dont leak memory

	freeMem();

	keys_ = src.keys_;
	values_ = src.values_;

	src.keys_ = nullptr;
	src.values_ = nullptr;
}

// copy assignment operator
FlatMap& FlatMap::operator=(const FlatMap& src) {
	doCopy(src, *this);

	return *this;
}

// move assignment operator
FlatMap& FlatMap::operator=(FlatMap&& src) noexcept {
	sz_ = src.sz_;
	cap_ = src.cap_;

	freeMem();

	keys_ = src.keys_;
	values_ = src.values_;

	src.keys_ = nullptr;
	src.values_ = nullptr;
	return *this;
}

Value& FlatMap::operator[](const Key& k) {
	FindResult res = findInd(k);

	if (std::get<1>(res)) {
		return values_[std::get<0>(res)];
	}
	else {
		Value def = Value();
		this->insert(k, def);

		return (*this)[k];
	}
}

bool operator==(const FlatMap& a, const FlatMap& b) {
	if (a.sz_ != b.sz_) { return false; }
	// cap can be inequal; doesn't matter

	for (size_t i = 0; i < a.sz_; i++) {
		if (a.keys_[i] != b.keys_[i]) {
			return false;
		}
		if (a.values_[i] != b.values_[i]) {
			return false;
		}
	}

	return true;
}

bool operator!=(const FlatMap& a, const FlatMap& b) {
	return !(a == b);
}