#pragma once

#pragma once

#include "../Simulation/Army.hpp"

inline void executeMilitaryStrategyForCountryAI(World& world, Country& country) {
	if (country.money <= 100) return;
	for (const auto& province : world.provinces) {
		if (province.owner != country.tag) continue;
		recruitArmy(world, country.tag, province.id);
		return;
	}
}

inline void executeMilitaryStrategyAI(World& world) {
	for (auto& country : world.countries) {
		if (country.tag == world.playerCountry) continue;
		executeMilitaryStrategyForCountryAI(world, country);
	}
}

