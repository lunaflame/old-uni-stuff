#include "gtest/gtest.h"
#include "../src/flatmap.h"

TEST(FlatMap_Util, Swap) {
	FlatMap fm1 = FlatMap(5);
	FlatMap fm2 = FlatMap(5);

	// fm1: { "ID": 1, "Key1", 69 }
	// fm2: { "ID": 2, "Key2", 100500 }

	fm1.insert("ID", 1);
	fm2.insert("ID", 2);

	fm1.insert("Key1", 69);
	fm2.insert("Key2", 100500);
	
	// fm1: { "ID": 2, "Key2", 100500 }
	// fm2: { "ID": 1, "Key1", 69 }

	fm1.swap(fm2);

	EXPECT_EQ(fm1.at("ID"), 2);
	EXPECT_EQ(fm2.at("ID"), 1);

	EXPECT_EQ(fm1.at("Key2"), 100500);
	EXPECT_EQ(fm2.at("Key1"), 69);

	// old keys shouldn't exist anymore
	EXPECT_THROW(fm1.at("Key1"), std::out_of_range);
	EXPECT_THROW(fm2.at("Key2"), std::out_of_range);
}

TEST(FlatMap_Util, CopyMove) {
	FlatMap fm1 = FlatMap(5);
	fm1.insert("ID", 1);

	// test copy: all should have ID = 1
	FlatMap fm2 = FlatMap(fm1); // by passing in ctor
	FlatMap fm3 = fm1;			// by assignment

	fm1.insert("UniqueKey", 1);
	fm2.insert("UniqueKey", 2);

	EXPECT_EQ(fm1.at("ID"), 1);
	EXPECT_EQ(fm2.at("ID"), 1);
	EXPECT_EQ(fm3.at("ID"), 1);

	EXPECT_EQ(fm1.at("UniqueKey"), 1);
	EXPECT_EQ(fm2.at("UniqueKey"), 2);
	EXPECT_ANY_THROW(fm3.at("UniqueKey"));

	// test move assignment
	// fm1 should be dead and moveInto should have all the values
	FlatMap moveInto;
	
	FlatMap copyAssignInto;
	FlatMap copyInto(fm1);

	EXPECT_EQ(copyInto.at("ID"), 1);
	EXPECT_EQ(copyInto.at("UniqueKey"), 1);

	FlatMap moveConstructInto = std::move(copyInto); // move a copy by construction

	copyAssignInto = fm1; // move original by assignment
	moveInto = std::move(fm1);

	EXPECT_EQ(moveInto.at("ID"), 1);
	EXPECT_EQ(moveInto.at("UniqueKey"), 1);
	
	EXPECT_EQ(copyAssignInto.at("ID"), 1);
	EXPECT_EQ(copyAssignInto.at("UniqueKey"), 1);

	EXPECT_ANY_THROW(fm1.at("ID")); // test fm1 being dead after being moved
}

TEST(FlatMap_Util, Equal) {
	FlatMap fm1 = FlatMap(5);
	FlatMap fm2 = FlatMap(fm1);
	
	// both are empty => equal
	EXPECT_TRUE(fm1 == fm2);

	fm1.insert("ID", 1);
	EXPECT_TRUE(fm1 != fm2);

	fm2.insert("ID", 1);
	EXPECT_TRUE(fm1 == fm2);

	fm2.erase("ID");
	fm2.insert("ID", 2);
	EXPECT_TRUE(fm1 != fm2);
}
