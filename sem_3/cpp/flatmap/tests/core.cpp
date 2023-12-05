#include "gtest/gtest.h"
#include "../src/flatmap.h"

// test if the dang thing even constructs properly
TEST(FlatMap_Core, CtorDtor) {
	FlatMap stack_fm = FlatMap();
	FlatMap initstack_fm = FlatMap(69);
}


TEST(FlatMap_Core, InsertGet) {
	// test basic functions working (insert + get)
	FlatMap fm = FlatMap(10);
	
	EXPECT_TRUE(fm.empty());
	EXPECT_FALSE(fm.contains("aaa"));

	// insert a bunch of k/v's
	EXPECT_TRUE(fm.insert("aaa", 1));
	EXPECT_TRUE(fm.insert("bbb", 2));
	EXPECT_TRUE(fm.insert("ccc", 3));
	
	EXPECT_FALSE(fm.empty());
	
	// try to replace an existing key-value (and fail)
	EXPECT_FALSE(fm.insert("bbb", 4));

	EXPECT_EQ(fm.at("aaa"), 1);
	EXPECT_EQ(fm["aaa"], 1);
	EXPECT_TRUE(fm.contains("aaa"));

	EXPECT_EQ(fm.at("ccc"), 3);
	EXPECT_EQ(fm["ccc"], 3);

	EXPECT_EQ(fm.at("bbb"), 2);
	EXPECT_EQ(fm["bbb"], 2);

	EXPECT_EQ(fm.size(), 3);

	// inserting at same index shouldn't increment size nor change value
	for (int i = 0; i < 10; i++) {
		fm.insert("aaa", i);
		EXPECT_EQ(fm.at("aaa"), 1);
	}

	EXPECT_EQ(fm.size(), 3);

	EXPECT_TRUE(fm.erase("aaa"));
	EXPECT_FALSE(fm.erase("no key"));
	EXPECT_EQ(fm.size(), 2);

	const FlatMap fmCopy = fm;

	EXPECT_EQ(fmCopy.at("bbb"), 2);
	EXPECT_ANY_THROW(fmCopy.at("abcde"));

	fm.clear();

	EXPECT_EQ(fm.size(), 0);
	EXPECT_TRUE(fm.empty());
	EXPECT_ANY_THROW(fm.at("aaa"));
}

TEST(FlatMap_Core, MissingKey) {
	// test calling `at` with missing key throwing an exception
	FlatMap fm = FlatMap(10);
	EXPECT_ANY_THROW(fm.at("some key"));

	// test calling `at` doesn't throw anymore after inserting the key
	fm.insert("some key", 1);
	EXPECT_EQ(fm.at("some key"), 1);

	// test if unsafe indexing creates default values
	Value some_value = fm["unsafe index"];
	EXPECT_EQ(fm.at("unsafe index"), some_value);

	fm["AAAAA"] = 100500;
	EXPECT_EQ(100500, fm["AAAAA"]);
}

TEST(FlatMap_Core, Resize) {
	// test flatmap resizing itself when necessary
	FlatMap fm = FlatMap(5);
	
	const char* randomKey = "pog";

	for (int i = 0; i < 1000; i++) {
		fm.insert(randomKey + std::to_string(i), i);
	}

	for (int i = 0; i < 1000; i++) {
		EXPECT_EQ(fm.at(randomKey + std::to_string(i)), i);
	}

	EXPECT_EQ(fm.size(), 1000);
}

int main(int argc, char** argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}