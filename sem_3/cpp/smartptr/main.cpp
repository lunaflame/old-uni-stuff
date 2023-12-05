#include "SmartPtr.h"
#include <iostream>
#include <string>


/*int main() {
	Tracker* trk = new Tracker("Main tracker");
	SmartPtr<Tracker> sp(trk);

	subFunc(sp);

	sp.SetAllowCopy(false);

	try {
		subFunc(sp);
	} catch (std::out_of_range oor) {
		std::cout << "Error while calling subfunc: " << oor.what() << std::endl;
	}
}
*/
