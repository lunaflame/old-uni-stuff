#include "gtest/gtest.h"
#include "../smartptr/SmartPtr.h" // where the hell is this thing

#define PRINT_TRACKERS false

class Tracker {
public:
	Tracker(std::string, bool* trk);
	~Tracker();

private:
	std::string id_;
	bool* trk_;
};

Tracker::Tracker(std::string id, bool* trk) {
	id_ = id;
	trk_ = trk;
	*trk = true;
	#if PRINT_TRACKERS
		std::cout << "	Tracker \"" << id_ << "\" initialized" << std::endl;
	#endif
}

Tracker::~Tracker() {
	*trk_ = false;

	#if PRINT_TRACKERS
		std::cout << "	Tracker \"" << id_ << "\" destroyed" << std::endl;
	#endif
}


void subFunc(SmartPtr<Tracker>& trkPtr) {
	bool origTrkExists;
	bool newTrkExists;

	{
		SmartPtr<Tracker> sp(trkPtr);								// second smartptr for tracker #1
		Tracker* trk2 = new Tracker("Subtracker", &newTrkExists);		// second tracker
		SmartPtr<Tracker> sp2(trk2);								// first smartptr for tracker 2
	}

	// both smartptr's should be destroyed, so:

	// trk2 should be destroyed
	EXPECT_FALSE(newTrkExists);

	//trk1 should remain
	EXPECT_TRUE(origTrkExists);
}

TEST(SmartPtr, Basic) {
	bool trkExists;
	Tracker* trk = new Tracker("Main tracker", &trkExists);

	{
		SmartPtr<Tracker> sp(trk);
		subFunc(sp);

		// `sp` still holds tracker, so it should still exist
		EXPECT_TRUE(trkExists);
	}
	// `sp` was the last SmartPtr holding trk, so trk should be destroyed
	EXPECT_FALSE(trkExists);
}

TEST(SmartPtr, DisableCopy) {
	bool trkExists;
	Tracker* trk = new Tracker("Main tracker", &trkExists);
	SmartPtr<Tracker> sp(trk);

	sp.SetAllowCopy(false);

	// smart pointers with copying disabled throw exceptions
	// note: that also means that the allocated resource will get leaked
	// there a proper solution to this?
	EXPECT_THROW(subFunc(sp), std::out_of_range);

	sp.SetAllowCopy(true);
	EXPECT_NO_THROW(subFunc(sp));
}