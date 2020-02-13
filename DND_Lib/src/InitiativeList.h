//
// Created by alexa on 8/23/2019.
//

#ifndef DNDIDEA_INITIATIVELIST_H
#define DNDIDEA_INITIATIVELIST_H

#include <string>
#include <vector>
#include <algorithm>

struct ThingA {
	int initiative{};
	std::string name{};

	bool operator==(const ThingA &rhs) const {
		return initiative == rhs.initiative &&
		       name == rhs.name;
	}

	bool operator!=(const ThingA &rhs) const {
		return !(rhs == *this);
	}
};

class InitiativeList {
private:
	std::vector<ThingA> participants{};
	size_t currIdx{};
public:
	void AddParticipant(ThingA participant) {
		participants.push_back(participant);
	}

	void RemoveParticipant(ThingA participant) {
		using namespace std;
		participants.erase(find(participants.begin(), participants.end(), participant), participants.end());
	}

	void NextPartipant() {
		currIdx = ++currIdx % participants.size();
	}

	ThingA CurrentParticipant() {
		if (!participants.empty()) {
			return participants.at(currIdx);
		}
		return ThingA{};
	}
};

#endif //DNDIDEA_INITIATIVELIST_H
