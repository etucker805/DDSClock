
#include <chrono>
#include <thread>
#include <cmath>

#include "robobrawlClock.hh"
#include "gui.hh"

// ---------------------------------------------------------------
bool roboClock::done = false;

Controller roboClock::control;
Context roboClock::context;
std::string roboClock::deviceId = "";
int roboClock::domain_id = 0;

// state encoding : -1 retain whatever was going on before. 0 is off- nothing. 1 - to zeros, 2 - full time, 3 - start run, 4 running, 5 paused, 6 resume,- 7 orange win - 8 blue win - 9 red for just in case.

static Timer timer_01hz; //  0.1 Hz
static Timer timer_1hz;  //  1   Hz
static Timer timer_10hz; // 10   Hz

// ---------------------------------------------------------------
void roboClock::main_loop()
{
  timer_01hz.start();
  timer_1hz.start();
  timer_10hz.start();
  SysName temp = SysName();
  temp.deviceId("hi ");
  temp.sysName("hp2");

  // org
  
  while (!done)
  {
    
    // every 0.1 sec:
    if (timer_01hz.elapsed().count() >= 50)
    {
      
      timer_01hz.start();
      // for gui frames.
      gui::loop(domain_id);
      
    }

    // every 1 sec:
    if (timer_1hz.elapsed().count() >= 300)
    {
      timer_1hz.start();
      // heartbeat.
      auto org = *roboClock::control.known_devices.getOrg();
      for (auto &pair : org)
      {

        ArenaCommand arena;
        ClockCommand comm;
        ButtonCommand orangeButton;
        ButtonCommand blueButton;

        timeValue time;
        float timeNum;

        std::string system = pair.first;
        SystemInfo info = pair.second;

        comm.sysName(system);
        arena.sysName(system);
        switch (info.state)
        {
        case State::WAITING: // pure waiting - arena white.  - do buttons  in gui for idel the system  and remove from idle.

        

          // not going to send messages - other than ambient I guess cause im gonna make that a ras pi... but I dont have those writers yet.
          control.WriteToBlueButtons(system, Colors::COLOR_BLACK);
          control.WriteToOrangeButtons(system, Colors::COLOR_BLACK);
          control.known_devices.stopTimer(system);

          //buttons off
          
          arena.color(Colors::COLOR_WHITE);
          control.writeToArena(arena);
          break;
        case State::WAIT_FOR_READY: // waiting for readys.. - arena  off ? ( add a start for idle and before match I think..... )

          // buttons on orange / blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);
          control.known_devices.stopTimer(system);

        //
          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_GREEN);       // red???/ green? idk man
          comm.blueGearColor(Colors::COLOR_BLACK);   // off
          comm.orangeGearColor(Colors::COLOR_BLACK); // off
          time.minutes(0);
          time.seconds(0);
          comm.time(time);
          control.writeToClock(comm); // ill be writing this ALOT... but for timeing ill do it anyway? just be good at cleanup on esp...

          arena.color(Colors::COLOR_BLACK);
          control.writeToArena(arena);

          break;
        case State::ORANGE_READY: // one is ready... orange - half orange / green???



          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_GREEN);

          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_RED);
          comm.blueGearColor(Colors::COLOR_BLACK);    // off
          comm.orangeGearColor(Colors::COLOR_ORANGE); // trun this one on
          time.minutes(0);
          time.seconds(0);
          comm.time(time);

          control.writeToClock(comm);

          break;
        case State::BLUE_READY: // blue is ready, - half colored ( gree / blue )


          // buttons on Orange: Orange / Blue:Green. 
          control.WriteToBlueButtons(system, Colors::COLOR_GREEN);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_RED);
          comm.blueGearColor(Colors::COLOR_BLUE);    // off
          comm.orangeGearColor(Colors::COLOR_BLACK); // trun this one on
          time.minutes(0);
          time.seconds(0);
          comm.time(time);

          comm.sysName(system);
          control.writeToClock(comm);

          break;
        case State::BOTH_READY: // both ready - green ? off ? white ? I think off - turn on at

          // buttons on Orange: green / Blue:Green. 
          control.WriteToBlueButtons(system, Colors::COLOR_GREEN);
          control.WriteToOrangeButtons(system, Colors::COLOR_GREEN);
          control.known_devices.stopTimer(system); // just makign sure.

          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_RED);
          comm.blueGearColor(Colors::COLOR_BLUE);     // off
          comm.orangeGearColor(Colors::COLOR_ORANGE); // trun this one on
          time.minutes(0);
          time.seconds(0);
          comm.time(time);

          comm.sysName(system);
          control.writeToClock(comm);

          break;
        case State::COUNTDOWN: // 3 - 2 - 1 

          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLACK);
          control.WriteToOrangeButtons(system, Colors::COLOR_BLACK);

          timeNum = 3 - (info.timer.elapsedMsec() / 1000); // division cause ms.
          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_YELLOW);

          time.seconds(int(timeNum) * 11); // shoudld go 33::33 -> 22:22 -> 11:11 -. start.
          time.minutes(int(timeNum) * 11);
          comm.time(time);
          control.writeToClock(comm);

          if (timeNum <= 0)
          {
            control.known_devices.setOrgState(system, State::RUN_START); 
            control.known_devices.stopTimer(system);

          }

          break;

        case State::RUN: // runnun.

          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          comm.isOff(0);
          comm.doDisplayTime(1);

          // so duration - clock timer -> get elapsed = remaining time in seconds, then do color check and display.
          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          if(timeNum <= 0 ){
            // go to a 000 match over state, dont know if I have.
            control.known_devices.setOrgState(system, State::MATCH_OVER); 
          }

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          
          control.writeToClock(comm); // should just work probaly.... need a resume state tho.

          break;
        case State::BLUE_WIN: // blue win 

          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_BLUE);

          time.seconds(88);
          time.minutes(88);

          comm.isOff(0);
          comm.blueGearColor(Colors::COLOR_BLUE);
          comm.orangeGearColor(Colors::COLOR_BLUE);
          comm.mainColor(Colors::COLOR_BLUE);

          comm.doDisplayTime(1);

          comm.time(time);
          control.writeToClock(comm);

          break;
        case State::ORANGE_WIN: // Orange Win

          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_ORANGE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          time.seconds(88);
          time.minutes(88);

          comm.isOff(0);
          comm.blueGearColor(Colors::COLOR_ORANGE);
          comm.orangeGearColor(Colors::COLOR_ORANGE);
          comm.mainColor(Colors::COLOR_ORANGE);

          comm.doDisplayTime(1);

          comm.time(time);
          control.writeToClock(comm);

          //control.known_devices.setOrgState(system, -1); 

          break;
        case State::ERROR: // AHHHHHHH i think 

          control.WriteToBlueButtons(system, Colors::COLOR_RED);
          control.WriteToOrangeButtons(system, Colors::COLOR_RED);
          control.known_devices.pauseTimer(system); // just in case..

          time.seconds(0);
          time.minutes(0);

          comm.isOff(0);
          comm.blueGearColor(Colors::COLOR_RED);
          comm.orangeGearColor(Colors::COLOR_RED);
          comm.mainColor(Colors::COLOR_RED);

          comm.doDisplayTime(1);

          comm.time(time);
          control.writeToClock(comm);

          break;

        case State::PAUSE: // pause it..... 

          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          control.known_devices.pauseTimer(system);
          comm.isOff(0);
          comm.doDisplayTime(1);

          // so duration - clock timer -> get elapsed = remaining time in seconds, then do color check and display.
          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          control.writeToClock(comm); // should just work probaly.... need a resume state tho. (12)
          // set state to 5
          control.known_devices.setOrgState(system, State::RUN);
          break;

        case State::COUNTDOWN_START: // 3-2-1 -  the starrt 

      
          control.known_devices.startTimer(system);

          timeNum = 3 - (info.timer.elapsedMsec() / 1000); // division cause ms.
          comm.isOff(0);
          comm.doDisplayTime(1);
          comm.mainColor(Colors::COLOR_YELLOW);

          time.seconds(int(timeNum) * 11); // shoudld go 33::33 -> 22:22 -> 11:11 -. start.
          time.minutes(int(timeNum) * 11);
          comm.time(time);
          control.writeToClock(comm);

          control.known_devices.setOrgState(system, State::COUNTDOWN); 

          break;
        case State::RUN_START: // run?

          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);


          control.known_devices.startTimer(system);
          comm.isOff(0);
          comm.doDisplayTime(1);

          // so duration - clock timer -> get elapsed = remaining time in seconds, then do color check and display.
          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          control.writeToClock(comm);
          

          // auto to state 5 (RUN)
          control.known_devices.setOrgState(system, State::RUN); 


          break;
        case State::RESUME: // resume
          control.known_devices.resumeTimer(system);
          control.known_devices.setOrgState(system, State::RUN); // put back into running but with a resumed timer. - can ony beapaused in the 3:00 state (5)
          control.writeToClock(comm);
          break;

        case State::MATCH_OVER: // match over. 

          //lights red, buttons stay( whatever I made them when running.), clock to red 0000 

          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          control.known_devices.stopTimer(system);

          comm.isOff(0);
          comm.doDisplayTime(1);

          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          control.writeToClock(comm);

          break;

        case State::UNSTICK_START:
          // do a pause... Do I need to have witch thininggigdig (orange or blu)?

          // buttons on Orange: green / Blue:blue. 
          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          control.known_devices.pauseTimer(system);
          comm.isOff(0);
          comm.doDisplayTime(1);

          // so duration - clock timer -> get elapsed = remaining time in seconds, then do color check and display.
          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          control.writeToClock(comm); // should just work probaly.... need a resume state tho. (12)
          // set state to 5
          control.known_devices.setOrgState(system, State::UNSTICK_ACTIVE); // unstick active 

          break;

        case State::UNSTICK_ACTIVE:
          // uh just wait till gui resolution. ( go to resume. )
          // do a display time state I guess.... 

          control.WriteToBlueButtons(system, Colors::COLOR_BLUE);
          control.WriteToOrangeButtons(system, Colors::COLOR_ORANGE);

          comm.isOff(0);
          comm.doDisplayTime(1);

          // so duration - clock timer -> get elapsed = remaining time in seconds, then do color check and display.
          timeNum = info.durationSec - (info.timer.elapsedMsec() / 1000); // division cause ms.

          if(timeNum <= 0 ){
            // go to a 000 match over state, dont know if I have.
            control.known_devices.setOrgState(system, State::MATCH_OVER); 
          }

          time.seconds(fmod(timeNum, 60));
          time.minutes(int(timeNum / 60));
          comm.time(time);
          
          control.writeToClock(comm); 



          break;


        default:
          std::cout << "unknown Clock state in: " << pair.first << std::endl;
          break;
        }

      }
      // control.writeHeartbeat();
    }

    // every 10 sec:
    if (timer_10hz.elapsed().count() >= 10000)
    {
      timer_10hz.start();
      // check for dead heartbeats.
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  
}
