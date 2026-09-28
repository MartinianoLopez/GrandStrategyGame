#include <iostream>
#include "../Model/World.hpp"
#include "../utils.hpp"

constexpr int INVEST_COST = 100;

inline bool Invest(World& world, const std::string& investor, int selected_province) {
    std::cout << "[Invest] investor=" << investor
              << " selected_province=" << selected_province << '\n';

    Country* c = findCountryByTag(world.countries, investor);
    Province* p = findProvinceById(world.provinces, selected_province);

    if (!c) {
        std::cout << "[Invest] pais no encontrado: " << investor << '\n';
        return false;
    }
    if (!p) {
        std::cout << "[Invest] provincia no encontrada: " << selected_province << '\n';
        return false;
    }

    std::cout << "[Invest] money=" << c->money
              << " owner=" << p->owner
              << " localEconomy=" << p->localEconomy << '\n';

    if (c->money < INVEST_COST) {
        std::cout << "[Invest] dinero insuficiente (" << c->money
                  << " < " << INVEST_COST << ")\n";
        return false;
    }
    if (p->owner != investor) {
        std::cout << "[Invest] la provincia pertenece a " << p->owner
                  << ", no a " << investor << '\n';
        return false;
    }

    c->money -= INVEST_COST;
    p->localEconomy += 1;

    std::cout << "[Invest] OK -> money=" << c->money
              << " localEconomy=" << p->localEconomy << '\n';
    return true;
}