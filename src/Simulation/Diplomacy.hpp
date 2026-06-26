#include <string>
#include "../Model/World.hpp"
#include "../utils.hpp"
#include "../Model/Loader.hpp"


inline void declareWar(World& world,std::string attacker, std::string defender){

    // relation
    Country* attackerCountry = findCountryByTag(world.countries, attacker);
    Country* defenderCountry = findCountryByTag(world.countries, defender);
    attackerCountry->addRelationship(Relationship(defender, TypeOfRelation::WAR));
    defenderCountry->addRelationship(Relationship(attacker, TypeOfRelation::WAR));

    // access
    attackerCountry->addAccesibleCountries(defender);
    defenderCountry->addAccesibleCountries(attacker);
    rechargeAccesibilityGraph(world, attackerCountry);
    rechargeAccesibilityGraph(world, defenderCountry);
    
}