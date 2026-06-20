#pragma once
#include "../Model/World.hpp"
#include "Army.hpp"
#include <string>
#include <cstdint>

struct Date { int year, month, day; };

// inverse of days_to_date — needed to compute the starting day count
constexpr int64_t days_from_civil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    int64_t era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = static_cast<unsigned>(y - era * 400);
    unsigned doy = (153*(m + (m > 2 ? -3 : 9)) + 2)/5 + d - 1;
    unsigned doe = yoe*365 + yoe/4 - yoe/100 + doy;
    return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

// world.days = 0 used to mean 1970-01-01; now it means this date instead
constexpr int64_t CAMPAIGN_START_DAYS = days_from_civil(1434, 4, 4);

inline std::string date_to_string(const Date& d) {
    static const char* const months[] = {
        "January","February","March","April","May","June",
        "July","August","September","October","November","December"
    };
    std::string s;
    s += std::to_string(d.day);
    s += ' ';
    s += months[d.month - 1];
    s += ' ';
    s += std::to_string(d.year);
    return s;
}

inline Date days_to_date(int64_t days) {
    days += 719468;
    int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    uint64_t doe = days - era * 146097;
    uint64_t yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;
    int64_t y = yoe + era * 400;
    uint64_t doy = doe - (365*yoe + yoe/4 - yoe/100);
    uint64_t mp = (5*doy + 2)/153;
    uint64_t d = doy - (153*mp+2)/5 + 1;
    uint64_t m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
    return { (int)y, (int)m, (int)d };
}

inline void updateWorld(World& world){
    updateArmyMovement(world);
}
inline void updateDate(World& world){
    world.date = date_to_string(days_to_date(world.days));
}

inline void timeRun(World& world, float deltaTime) {
    world.timeAccumulator += deltaTime * world.timeSpeed;
    if (world.timeAccumulator >= 1.0f) {
        world.timeAccumulator -= 1.0f;
        world.days += 1;
        updateDate(world);
        updateWorld(world);
    }
}