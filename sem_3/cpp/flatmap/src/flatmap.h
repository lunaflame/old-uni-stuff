#ifndef Flatmap_IG
#define Flatmap_IG

#include <string>

typedef std::string Key;

/*struct Value {
	unsigned age;
	unsigned weight;
};
*/

typedef int Value;

class FlatMap
{
public:
	static constexpr size_t DefaultCap = 50; // 0 = death
	static constexpr float GrowRatio = 2;	// grow capacity by this factor every time we need to
											// grow ratio equal to or less than 1 = death

	// Default constructor uses `DefaultCap` as the initial capacity.
	FlatMap();

	// Uses `cap` as the initial capacity.
	FlatMap(size_t cap);
	~FlatMap();

	FlatMap(const FlatMap& b);
	FlatMap(FlatMap&& b);

	FlatMap& operator=(const FlatMap& b);
	FlatMap& operator=(FlatMap&& b) noexcept;

	// Swaps two flatmaps' contents and properties (such as capacity and size).
	void swap(FlatMap& b);

	// Clear the contents of the container.
	void clear();

	// Erase the element by key.
	// Returns false if no such key was found in the first place, true if found and erased.
	bool erase(const Key& k);

	// Insert a key-value pair.
	// Returns false if this key-value pair already existed, true if inserted successfully.
	bool insert(const Key& k, const Value& v);

	// Check if a key-value pair existed.
	// Returns true if it did, false otherwise.
	bool contains(const Key& k) const;

	// Index by key; if the key-value existed, returns the value
	// If the key-value didn't exist, creates a new value using the default constructor,
	// and inserts a new key-value.
	// Returns the reference to the value from the key-value.
	Value& operator[](const Key& k);

	// Index by key; returns the value if a key-value existed.
	// Throws an exception otherwise.

	Value& at(const Key& k);
	const Value& at(const Key& k) const;

	// Return the current size of the flatmap (how many key-values are inserted?)
	size_t size() const;

	// Return whether the flatmap has any key-values in it
	bool empty() const;

	// Compares two flatmaps' contents; if every key-value matches, returns true.
	friend bool operator==(const FlatMap& a, const FlatMap& b);

	// Inverse of ==
	friend bool operator!=(const FlatMap& a, const FlatMap& b);

private:
	Key* keys_ = nullptr; // ew no vectors!!!!!!!
	Value* values_ = nullptr;

	size_t sz_ = 0;
	size_t cap_ = 0;

	void incrSize();
	void growSize(size_t newCap);

	std::tuple<size_t, bool> findInd(const Key& key) const;

	void doCopy(const FlatMap& from, FlatMap& to);
	void freeMem();
};

// constexpr size_t FlatMap::DefaultCap;
// constexpr float FlatMap::GrowRatio;	

#endif