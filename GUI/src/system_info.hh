#include <map>
#include <string>
#include "robobrawl.hh"
#include "timer.hh"

//setttings definitiaons. 
#define DO_3_SEC_COUNTDOWN 0x0001 << 0



enum class State{
    WAITING = -1,
    WAIT_FOR_READY = 0,
    ORANGE_READY = 1,
    BLUE_READY = 2,
    BOTH_READY = 3,
    COUNTDOWN = 4,
    RUN = 5,
    BLUE_WIN = 6,
    ORANGE_WIN = 7,
    ERROR = 8,
    PAUSE = 9, // not certain this is pause hold of pause starrt
    COUNTDOWN_START = 10,
    RUN_START = 11,
    RESUME = 12,
    MATCH_OVER = 13, // as in time expires I think. 
    UNSTICK_START = 14,
    UNSTICK_ACTIVE = 15

    // to ADD: TAPOUTS ( might just pause state it and call it a day???? )
};

struct SystemInfo
{

    std::map<DeviceRole, std::vector<std::string>> devices;

    float durationSec;
    NewTimer timer;
    uint32_t settings;
    State state;
    bool doCountdown; 


    /* data */
};


 