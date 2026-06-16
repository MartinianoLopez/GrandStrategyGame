#include "../Model/World.hpp"
/*
// ============================================================
// ARMY MOVEMENT SYSTEM
// ============================================================

// --- DATA STRUCTURES ---
Army {
    position
    name
    owner
    troops
    path[]     // TODO
    moveStage  // TODO
}

Province {
    id
    owner
    controller //TODO
    center
    siege  //TODO
    battle //TODO
}

// ============================================================
// PATH PLANNING
// ============================================================

calculatePath(fromProvince, toProvince):
    return aStar(fromProvince, toProvince)

createArmyMovement(fromProvince, toProvince):
    army = findArmyInProvince(fromProvince)
    if not army or army.troops <= 0: return
    path = calculatePath(fromProvince, toProvince)
    if path is empty: return
    army.path = path

// ============================================================
// DAILY SIMULATION TICK
// ============================================================

updateTroopMovement():
    for each army:
        if army.path is empty: continue
        army.moveStage += 15 // no modifiers for now
        if army.moveStage >= 100:
            army.moveStage -= 100
            army.position = army.path.popFirst()
            checkForCollisions(army)

// ============================================================
// COLLISION & COMBAT RESOLUTION
// ============================================================

checkForCollisions(army):
    for each otherArmy at same position:
        if enemy and atWar:
            checkForBattles(army, otherArmy)
    if province.controller != army.owner and atWar:
        startSiege(army, province)


checkForBattles(army, otherArmy):
    if province.hasBattle:
        joinBattle(army)
    else:
        startBattle(army, otherArmy)

startBattle(army, otherArmy){
    // automatic battle for now
    army.troops.multiplied by random from 1 to 2 >= otherarmy.troops multiplied by random from 1 to 2
    winner is bigger
    calculate engagement mortality for loser()
    loser army.troops -> loser.troops - loser.troops*mortality
    calculate engagement mortality for winner()
    winer army.troops -> winner.troops - winner.troops*mortality
}
    
startSiege(army, province):
    // automatic siege for now
    province.controller = army.owner
    redrawCountriesImg()

// ============================================================
// RENDERING
// ============================================================

showTroopPath(selectedArmy):
    if not selectedArmy: return
    prev = selectedArmy.position.center
    for each province in selectedArmy.path:
        drawLine(prev, province.center)
        prev = province.center
*/
inline void moveArmy(Army& army, int toProvinceId) {
    army.position = toProvinceId;
}

inline Army* armyPositionFind(const std::list<Army>& list, int provinceId) {
    for (auto& army : list)
        if (army.position == provinceId)
            return const_cast<Army*>(&army);
    return nullptr;
}
