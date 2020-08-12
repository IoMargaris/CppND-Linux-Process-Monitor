#include <string>

#include "format.h"

using std::string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {

    int intSeconds = (seconds % 60);
    int minutes = (seconds % 3600) / 60;
    int hours = seconds / 3600;

    string ElapsedTime = std::to_string(hours) + ":" + std::to_string(minutes) + ":" + std::to_string(intSeconds);
    return ElapsedTime;

}