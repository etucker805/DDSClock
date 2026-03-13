#include "controller.hh"


void Controller::handle_button_data(const ButtonData & data){
    // update button states. 
    //std::cout << " oohohoh a button data" << std::endl;

    //oka soooooooo 
    if(!data.mainPressed() && !data.tapoutPressed() ){
        // no action happened - likey uninteded message - bug in button . ( or they pressed both imma ignore it..) 
        // might log here
    }
    else
    {
        // I have system and devId - so look at blue / orange, need a system state I think... so I can change that and update the clock state
        auto org = *known_devices.getOrg();
        if(data.mainPressed() != 0){
            // switch over this system state..

            // THIS IS::: If main press from either button 
            switch(org[data.sysName()].state){
                // 0: idle, 
                // No inputs in waiting state
                case State::WAIT_FOR_READY :
                    if(known_devices.get_device_role(data.deviceId( )) == DeviceRole::ROLE_BUTTON_BLUE){
                        known_devices.setOrgState(data.sysName(), State::BLUE_READY); 
                    } else {
                        //orange button
                        known_devices.setOrgState(data.sysName(), State::ORANGE_READY); 
                    }
                    break;
                case State::ORANGE_READY:
                    if(known_devices.get_device_role(data.deviceId( )) == DeviceRole::ROLE_BUTTON_BLUE){
                        known_devices.setOrgState(data.sysName(), State::BOTH_READY); 

                    } else{
                        //orange repeat press...
                    }
                    break;
                case State::BLUE_READY:
                    if(known_devices.get_device_role(data.deviceId( )) == DeviceRole::ROLE_BUTTON_ORANGE){
                        known_devices.setOrgState(data.sysName(), State::BOTH_READY); 

                    } else{
                        //blue repeat press...
                    }
                    break;
                // others I now care about for MAIN press: ( mostly for unstick )
                // run.. (run start? no.. )
                // NOT: 3-2-1, pause, end, wins, uh hmmmm  

                case State::RUN : 
                    if(known_devices.get_device_role(data.deviceId( )) == DeviceRole::ROLE_BUTTON_ORANGE){
                        // ORANGE unstick ( might just trigger a pause and set a flag to display on gui.. ( special pause for unstick at least I think..... )) 
                    } else {
                        // Blue Unstick
                    }




                    break;
                default:
                    //unknown state???????? wth. 
                    break;
            } 

            printf("New State %d\n", org[data.sysName()].state);
        } else if( data.tapoutPressed() != 0){
            switch(org[data.sysName()].state){
                // 0: idle, 1: orangReady, 2: blueReady, 3: goTime/Running, 4: doneIdle, 5: orangeWInn, 6: BlueWin
               
                case State::RUN:
                    if(known_devices.get_device_role(data.deviceId( )) == DeviceRole::ROLE_BUTTON_BLUE){
                        known_devices.setOrgState(data.sysName(), State::RUN); 
                    } else {
                        //orange button
                        known_devices.setOrgState(data.sysName(), State::BLUE_WIN); 
                    }
                    break;
                // I think I only care here... Ill ask uday or someone latter I think... I just need to have a bunck of questions in a list to semd him about what he wants to do. 
                default:
                    //unknown state???????? wth. 
                    break;
            }   
        }
    }

}

void Controller::handle_heartbeat( const Heartbeat & data ){
    // update livelinesss.
    std::cout << "wowsa\n";
    // I will actualy need to make an alg for this.... huhhhhhhhhhh I dont wannnnnannnananannan ahhhhhh
}