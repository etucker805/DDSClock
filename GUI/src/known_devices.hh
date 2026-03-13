

#ifndef DEVICES_HH
#define DEVICES_HH

#include <map>
#include "robobrawl.hh"
#include "system_info.hh"
#include "context.hh"

class KnownDevices : public dds::sub::NoOpDataReaderListener<DeviceInfo>
{

public:
  // -------------------------------------------------------
  KnownDevices() : dr(dds::core::null),
                   known_devices()
  {
  }

  // -------------------------------------------------------
  ~KnownDevices()
  {
  }

  // -------------------------------------------------------
  void
  init(Context &context)
  {
    auto topic = dds::topic::Topic<DeviceInfo>(context.participant(), "DeviceInfo");
    dds::sub::qos::DataReaderQos dr_qos;
    context.subscriber().default_datareader_qos(dr_qos); // Get default QoS
    dr_qos.policy<dds::core::policy::Reliability>(dds::core::policy::ReliabilityKind::RELIABLE); // Apply the Durability QoS
    dr = dds::sub::DataReader<DeviceInfo>(context.subscriber(), topic, dr_qos);
    
    dr.listener(this, dds::core::status::StatusMask::all());
  }

  // -------------------------------------------------------
  void clear()
  {
    dr.listener(NULL, dds::core::status::StatusMask::all());
    dr = dds::core::null;
  }

  // -------------------------------------------------------
  std::vector<std::string> known_device_ids()
  {
    std::vector<std::string> ids;
    for (auto const &imap : known_devices)
    {
      ids.push_back(imap.first);
    }
    return ids;
  }

  // -------------------------------------------------------
  const DeviceInfo &get_deviceinfo(std::string devId)
  {
    auto search = known_devices.find(devId);
    if (search != known_devices.end())
      return search->second;
    else
      throw std::runtime_error("unknown");
  }

  // -------------------------------------------------------
  DeviceRole get_device_role(std::string devId)
  {
    auto search = known_devices.find(devId);
    if (search != known_devices.end())
      return search->second.role();
    else
      return DeviceRole::ROLE_UNKNOWN;
  }

  std::map<std::string, SystemInfo>* getOrg()
  {
    return (&org);
  }

  void setOrgState(std::string sysName, State state){
    org[sysName].state = state;
  }

  void setOrgDuration(std::string sysName, float duration){
    org[sysName].durationSec = duration;
  }

  void setDoCountdown(std::string sysName, bool doCountdown){
    org[sysName].doCountdown = doCountdown;
  }

  void startTimer(std::string sysName){
    org[sysName].timer.start();
  }

  void pauseTimer(std::string sysName){
    org[sysName].timer.pause();
  }

  void stopTimer(std::string sysName){
    org[sysName].timer.stop();
  }

  void resumeTimer(std::string sysName){
    org[sysName].timer.resume();
  }

  int64_t getTimerElapsedMsec(std::string sysName){
    return org[sysName].timer.elapsedMsec();
  }

  // -------------------------------------------------------
  void handle_deviceinfo(const DeviceInfo &devinfo)
  {
    auto result = known_devices.insert(std::make_pair(devinfo.deviceId(), devinfo));
    // const auto [it, added] =
    std::cout << "Got:  DeviceInfo\n";
    auto it = result.first;
    auto added = result.second;
    if (!added)
    {
      it->second = devinfo;
    }
    else
    {
      fprintf(stderr, "ADD  DEVICE: '%s'\n", devinfo.deviceId().c_str());
    }
    // okay add to our org...
    // look fro sys,
    // yes:: find lists and  addd the id
    // NO:: make empty and add

    if (org.find(devinfo.sysName()) != org.end())
    {

      // has System. ??
      if(std::find(org[devinfo.sysName()].devices[devinfo.role()].begin(), org[devinfo.sysName()].devices[devinfo.role()].end(), devinfo.deviceId()) == org[devinfo.sysName()].devices[devinfo.role()].end()){
        org[devinfo.sysName()].devices[devinfo.role()].push_back(devinfo.deviceId());
      } 

    }
    else
    {

      // no sysname of hta found
      std::map<DeviceRole, std::vector<std::string>> tempMap;
      // inset vevctors into it??
      tempMap[DeviceRole::ROLE_BUTTON_BLUE] = {};
      tempMap[DeviceRole::ROLE_BUTTON_ORANGE] = {};
      tempMap[DeviceRole::ROLE_CLOCK] = {};

      tempMap[devinfo.role()].push_back(devinfo.deviceId());

      SystemInfo tempInfo;
      tempInfo.devices = tempMap;
      tempInfo.durationSec = 180; // default 3 minuetts. 
      tempInfo.settings = 0; // nothing enabled I guesss... I may add a defualt one. 
      tempInfo.state = State::WAITING ; // set to Idle / wait for ready 
      tempInfo.timer = NewTimer();
      tempInfo.doCountdown = 1; 

      org.insert(std::make_pair(devinfo.sysName(), tempInfo));
    }

    // so how do I remove the device ( by devId ) from that system, and then check for the uh empty sytem... and where. 
    // uh looop and checlk in all systems != curr sys, and if devId in that roole, tthen remove, then check that tlist when I do that to see if removal.... I think 
    for(auto sysPair : org){
      // have [sysName, map<role, vect<dedvIds>> ] I think
      //so 
      for( auto it = sysPair.second.devices[devinfo.role()].begin(); it != sysPair.second.devices[devinfo.role()].end(); ){
        if( sysPair.first != devinfo.sysName() && *it == devinfo.deviceId()){
          // this one is in the wroong place..... 
          sysPair.second.devices[devinfo.role()].erase(it);
        } else {
          ++it;
        }
      }
    
      if(sysPair.second.devices[DeviceRole::ROLE_BUTTON_BLUE].size() +
       sysPair.second.devices[DeviceRole::ROLE_BUTTON_ORANGE].size() 
      + sysPair.second.devices[DeviceRole::ROLE_CLOCK].size() 
      + sysPair.second.devices[DeviceRole::ROLE_UNKNOWN].size() == 0){
        //mothing in that stystem. 
        std::cout << "empty sys erase\n";
        org.erase(sysPair.first);
        break;
      }
    }
    



  }

  // -------------------------------------------------------
  void remove_deviceinfo(const DeviceInfo &devinfo)
  {
    fprintf(stderr, "DEAD DEVICE: '%s'\n", devinfo.deviceId().c_str());
    known_devices.erase(devinfo.deviceId());
  }

  // -------------------------------------------------------
  void on_data_available(dds::sub::DataReader<DeviceInfo> &reader)
  {
    dds::sub::LoanedSamples<DeviceInfo> samples = reader.take();
    for (const auto &sample : samples)
    {
      if (sample.info().valid())
      {
        handle_deviceinfo(sample.data());
      }
      else
      {
        remove_deviceinfo(sample.data()); // key is valid, but no other fields
      }
    }
  }

protected:
  dds::sub::DataReader<DeviceInfo> dr;
  std::map<std::string, DeviceInfo> known_devices;

  std::map<std::string, SystemInfo> org;
  // so org[sysName][deviceRole] -> list of those device's ids in that system to then look at known devices fro more info if needed;
};

#endif
