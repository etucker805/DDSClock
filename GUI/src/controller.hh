// aight
#include "context.hh"
#include <string>

#include "button_data.hh"
#include "button_command.hh"
#include "clock_command.hh"
#include "arena_command.hh"
#include "heartbeat.hh"
#include "set_sysName.hh"
#include "known_devices.hh"

class Controller : public ButtonDataClient,
                   public HeartbeatClient
{
protected:
    /* data */
    ButtonDataReader buttonData;

    ButtonCommandWriter buttonWriter;
    ClockCommandWriter clockWriter;
    ArenaCommandWriter arenaWriter;
    HeartbeatWriter heartbeat;
    SetSysName setSysName;

    std::string devId;

public:
    // keyeeeesss?
    // states: idle, before ready type thing, then the readys - a blu eready and orang and a both-> auto transitioj to running or just the running
    // running -> contians all the paused, resume, running ect.
    // some kind of doneIdle -> at 0000 no decition tho (diff than Idle? )
    // winner states.
    // 0: idle, 1: orangReady, 2: blueReady, 3: goTime/Running, 4: doneIdle, 5: orangeWInn, 6: BlueWin
    KnownDevices known_devices;

    Controller(/* args */) {};
    ~Controller() {};

    void init(Context &context, std::string deviceId)
    {
        this->devId = deviceId;

        known_devices.init(context);

        buttonData.init(this, context);

        // heartbeat.init( this, context, deviceId);

        buttonWriter.init(context, devId);
        clockWriter.init(context, devId);
        arenaWriter.init(context, deviceId);
        setSysName.init(context, deviceId);
    };

    void startup()
    {
        // device info, idk what else.
    }

    void writeToClock(ClockCommand &command)
    {
        clockWriter.publish(command);
    }

    void writeToButton(ButtonCommand &command)
    {
        buttonWriter.publish(command);
    }

    void WriteToBlueButtons(std::string sysName, Colors color)
    {
        auto org = *known_devices.getOrg();
        ButtonCommand command;
        for (auto id : org[sysName].devices[DeviceRole::ROLE_BUTTON_BLUE])
        {
            command.deviceId(id);
            command.buttonState(color);
            buttonWriter.publish(command);
        }
    }

    void WriteToOrangeButtons(std::string sysName, Colors color)
    {
        auto org = *known_devices.getOrg();
        ButtonCommand command;
        for (auto id : org[sysName].devices[DeviceRole::ROLE_BUTTON_ORANGE])
        {
            command.deviceId(id);
            command.buttonState(color);
            buttonWriter.publish(command);
        }
    }

    void writeToArena(ArenaCommand &command)
    {
        arenaWriter.publish(command);
    }

    void writeSysName(SysName &command)
    {
        setSysName.publish(command);
    }

    void writeHeartbeat()
    {
        heartbeat.publish();
    }

    void clear()
    {
        buttonWriter.clear();
        clockWriter.clear();
        known_devices.clear();
    };


    virtual void handle_button_data(const ButtonData &data);
    virtual void handle_heartbeat(const Heartbeat &data);
};



