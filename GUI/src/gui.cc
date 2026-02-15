
#include "gui.hh"
#include <iostream>
#include <chrono>
#include <map>
#include <vector>
#include <string>
#include <array>

#define USE_GLFW // remove befreo compileing ( cmake handles os specific backend, vs is ignorant of that )

#if defined(USE_SDL2)
#include "sdl2.cc"
#elif defined(USE_GLFW)
#include "glfw.cc"
#elif defined(USE_DX11)
#include "dx11.cc"

#else
#error A imgui backend has not been defined
#endif

/*
#include "fonts.hh"
*/

#include "icon.hh"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

extern int participantNum;

float gui::dpi_scale = 0.5;
int gui::m_win_w = 0;
int gui::m_win_h = 0;
ImVec4 gui::clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
float gui::m_fixedH = 0.0f;
float gui::m_fixedW = 0.0f;
char gui::newSys[256] = "";
float gui::buffer = 5;

float dummyDur;

int button_cammand_state_item_number = 0;
int clock_cammand_state_item_number = 0;


std::map<std::string, Colors> state_names_to_color_class = {
    {"BLACK", Colors::COLOR_BLACK},
    {"BLUE", Colors::COLOR_BLUE},
    {"GREEN", Colors::COLOR_GREEN},
    {"ORNAGE", Colors::COLOR_ORANGE},
    {"RED", Colors::COLOR_RED},
    {"YELLOW", Colors::COLOR_YELLOW}

};

const char *state_names_list[] = {
    "BLACK",
    "BLUE",
    "GREEN",
    "ORANGE",
    "RED",
    "YELLOW",
};

ImFont *font2;

static uint8_t *iconPx;
static int iconX;
static int iconY;

std::map<std::string, bool> controlWindows;

// -----------------------------------------------------------------------------
static void
setup_dpi_scale(float scale)
{

  // fonts::LoadFonts( scale );
  ImGuiIO &io = ImGui::GetIO();
  ImFont *font1 = io.Fonts->AddFontFromFileTTF("imgui-1.90.9/src/imgui/misc/fonts/DroidSans.ttf", 50 * scale);
  font2 = io.Fonts->AddFontFromFileTTF("imgui-1.90.9/src/imgui/misc/fonts/DroidSans.ttf", 40);
  // ImFont* largeFont = io.Fonts->AddFontFromFileTTF("C:\\Users\\Ethan\\Desktop\\Guages\\src\\src\\test_sans.ttf", 50);
  // ImFont* smallFont = io.Fonts->AddFontFromFileTTF("C:\\Users\\Ethan\\Desktop\\Guages\\src\\src\\test_sans.ttf", 5);

  // io.Fonts->AddFontDefault();

  auto &style = ImGui::GetStyle();
  style = ImGuiStyle();
  ImGui::StyleColorsDark();

  style.WindowBorderSize = 1.f * scale;
  style.FrameBorderSize = 1.f * scale;
  style.FrameRounding = 0.f * scale;
  style.ScaleAllSizes(scale);

  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(1, 1, 1, 0.03f);
  style.Colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
  style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
}

// -----------------------------------------------------------------------------
void gui::init_backend(const char *win_title)
{
  backend::init_backend(win_title, 4000, 2000);
}

// -----------------------------------------------------------------------------
void gui::init_imgui()
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  setup_dpi_scale(gui::dpi_scale);

  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();
}

// -----------------------------------------------------------------------------
void gui::init_renderer()
{
  backend::init_renderer();
}

// -----------------------------------------------------------------------------
void gui::set_icon()
{
  iconPx = stbi_load_from_memory((const stbi_uc *)Icon_data, Icon_size,
                                 &iconX, &iconY, nullptr, 4);
  backend::set_icon(iconPx, iconX, iconY);
}

// -----------------------------------------------------------------------------
void gui::add_system(std::string sysName)
{
  controlWindows.insert(std::make_pair(sysName, false));
  // and hother things that I need to do I guess..
}

// -----------------------------------------------------------------------------
/* called each time through 'main_loop' to refresh gui
 * and collect/handle user input
 */
void gui::loop(int domain_id)
{

  static int counter = 0;

  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard
  //  flags to tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input
  //   data to your main application, or clear/overwrite your copy of
  //   the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard
  //   input data to your main application, or clear/overwrite your
  //   copy of the keyboard data.
  // Generally you may always pass all inputs to dear imgui, and hide
  // them from your application based on those two flags.
  backend::poll_events();

  backend::new_frame();
  ImGui::NewFrame();

  if (counter > 2)
  {
    // ImGui::PushFont( fonts::s_fixedWidth );
    gui::m_fixedW = ImGui::CalcTextSize("X").x;
    gui::m_fixedH = ImGui::CalcTextSize("X").y;
    // ImGui::PopFont( );

    // fill the entire 'window'
#ifdef IMGUI_HAS_VIEWPORT
    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->GetWorkPos());
    ImGui::SetNextWindowSize(viewport->GetWorkSize());
    ImGui::SetNextWindowViewport(viewport->ID);
#else
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
#endif

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::Begin("StackLight",
                 nullptr,
                 ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoTitleBar |
                     ImGuiWindowFlags_NoResize |
                     //ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoFocusOnAppearing |
                     ImGuiWindowFlags_NoMove |
                     // ImGuiWindowFlags_NoDocking |
                     ImGuiWindowFlags_NoNavFocus);

    // GUI CONTENTS:
    gui::draw_dashboard(domain_id);

    ImGui::End();
    ImGui::PopStyleVar(1);
  }
  else
  {
    counter++;
  }

  // ImGui::ShowDemoWindow();
  // ImPlot::ShowDemoWindow();

  // Rendering
  ImGui::Render();
  backend::render();
}

// -----------------------------------------------------------------------------
void gui::close_renderer()
{
  backend::close_renderer();
}

// -----------------------------------------------------------------------------
void gui::close_imgui()
{
  ImGui::DestroyContext();
}

// -----------------------------------------------------------------------------
void gui::close_backend()
{
  backend::close_backend();
}

void putTextInBox(ImDrawList *drawlist, ImVec2 pMin, ImVec2 pMax, const char text[], ImU32 color = IM_COL32(0, 0, 0, 255))
{
  ImVec2 pos = pMin;
  pos.y += ((pMax.y - pMin.y) / 2) - (ImGui::CalcTextSize(text).y / 2);
  pos.x += (pMax.x - pMin.x) / 2 - ImGui::CalcTextSize(text).x / 2;

  drawlist->AddText(pos, color, text);
}

void gui::draw_dashboard(int domain_id)
{

  // std::vector<std::string> ids = roboClock::known_devices.known_device_ids();

  // for(std::vector<std::string>::iterator it = ids.begin(); it != ids.end(); it++){
  //   std::cout << "AHHHHHH";
  //   ImGui::Text((*it).c_str());
  // }
  // std::cout << "\n";

  // I dont have any devices tooo test this against so imma start on the sesps.
  // i Think im gonna do this wiht beins
  std::map<std::string, SystemInfo> org = *roboClock::control.known_devices.getOrg();
  ImGui::Begin("Systems");
  {

    for (auto &pair : org)
    {
      ImGui::Text(pair.first.c_str());
      ImGui::SameLine();
      std::string buttonText = pair.first + " control";
      if (ImGui::Button(buttonText.c_str()))
      {
        controlWindows[pair.first] = !controlWindows[pair.first];
      }
    }
  }
  ImGui::End();

  for (auto &pair : controlWindows)
  {
    if (pair.second)
    {
      std::string system_name = pair.first;
      // ----------- SYSTEM WINDOW ----------------------------------
      // window is open
      
 

      ImGui::Begin("SystemControol");
      
      // -------------- SETTINGS ------------------------------------
      if(ImGui::CollapsingHeader("Settings:")){
        ImGui::Text("Info: ");

        // Num Orange buttons
        ImGui::Text(" - Number of Orange Buttons: %d", org[system_name].devices[DeviceRole::ROLE_BUTTON_ORANGE].size());

        // Num Blue
        ImGui::Text(" - Number of Blue Buttons: %d", org[system_name].devices[DeviceRole::ROLE_BUTTON_BLUE].size());

        // Num clock
        ImGui::Text(" - Number of Clocks: %d\n", org[system_name].devices[DeviceRole::ROLE_CLOCK].size());
        
        //state
        ImGui::Text(" - State: %d", org[system_name].state);

        // Clock Time
        ImGui::Text(" - Clock Duration: "  );
        ImGui::SameLine();
        ImGui::SetNextItemWidth(100);
        ImGui::InputFloat("##duration", &(org[system_name].durationSec));

        roboClock::control.known_devices.setOrgDuration(system_name, org[system_name].durationSec);
        


      }
      

      // --------------------------- BUTTONS -------------------------------
      if (ImGui::CollapsingHeader(" BUTTONS "))
      {
        int i = 0;

        // ------------------------- BLUE BUTTONSS -------------------------------------------

        for (auto it = org[system_name].devices[DeviceRole::ROLE_BUTTON_BLUE].begin(); it != org[system_name].devices[DeviceRole::ROLE_BUTTON_BLUE].end(); it++)
        {

          ImGui::Text((*it).c_str());
          ImGui::SameLine();
          ImGui::PushID(i);
          ImGui::PushStyleColor(ImGuiCol_Button, blue_col);
          if (ImGui::Button("Button Settings"))
          {
            ImGui::OpenPopup((*it).c_str());
          }
          ImGui::PopStyleColor();
          ImGui::SetNextWindowSize(ImVec2(400, 400));
          std::string deviceId = *it;
          // -------------------- BUTTON SETTINGS -----------------------------------------------
          if (ImGui::BeginPopupModal((*it).c_str(), nullptr, ImGuiWindowFlags_NoResize))
          {

            {
              // -------------------- SYS CHANGE WINDOW -------------------------------------
              if (ImGui::Button("send SysChange"))
              {
                ImGui::OpenPopup("SystemName");
              }
              if (ImGui::BeginPopupModal("SystemName"))
              {
                ImGui::InputText("New Sys Name", newSys, IM_ARRAYSIZE(newSys));

                if (ImGui::Button("Send & Close"))
                {
                  SysName msg = SysName();
                  msg.init();
                  msg.deviceId(deviceId);
                  msg.sysName(newSys);
                  std::cout << "write new SysName";
                  roboClock::control.writeSysName(msg);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                {
                  ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
              }

              // --------------------- BUTTON COMMAND WINDOW -----------------
              if (ImGui::Button("Send ButtonComand"))
              {
                ImGui::OpenPopup("ButtonCommand");
              }
              if (ImGui::BeginPopupModal("ButtonCommand"))
              {
                ImGui::Text("Set values for ButtonCommand");
                // DO THIS STUFFFFFFF TODO
                ImGui::Combo("State", &button_cammand_state_item_number, state_names_list, IM_ARRAYSIZE(state_names_list));
                if (ImGui ::Button("Close and Send"))
                {

                  ButtonCommand comm = ButtonCommand();
                  comm.deviceId(deviceId);
                  std::string color_name = state_names_list[button_cammand_state_item_number];
                  comm.buttonState(state_names_to_color_class[color_name]);
                  roboClock::control.writeToButton(comm);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                  ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
              }
              // ImGui::SetCursorPos(ImVec2(400 - ImGui::CalcTextSize("Close").y, 0));
              ImGui::SetCursorPos(ImVec2(buffer, 400 - ImGui::CalcTextSize("Close").y - buffer));

              if (ImGui::Button("Close This One"))
                ImGui::CloseCurrentPopup();

              ImGui::EndPopup();
            }
          }
          ImGui::PopID();
          // add buttons and stuff for the buttons. ( to send sys change message at least.. nick probably too.)

          i++;
        }
        i = 0;

        //---------------------------- ORANGGWE BUTTONS ------------------------------------------

        for (auto it = org[system_name].devices[DeviceRole::ROLE_BUTTON_ORANGE].begin(); it != org[system_name].devices[DeviceRole::ROLE_BUTTON_ORANGE].end(); it++)
        {
          ImGui::Text((*it).c_str());
          ImGui::SameLine();
          ImGui::PushID(i);
          ImGui::PushStyleColor(ImGuiCol_Button, orange_col);
          if (ImGui::Button("Button Settings"))
          {
            ImGui::OpenPopup((*it).c_str());
          }
          ImGui::PopStyleColor();
          ImGui::SetNextWindowSize(ImVec2(400, 400));
          std::string deviceId = *it;
          if (ImGui::BeginPopupModal((*it).c_str(), nullptr, ImGuiWindowFlags_NoResize))
          {

            { // all teh windows inn a space
              // -------------------- SYS CHANGE WINDOW -------------------------------------
              if (ImGui::Button("send SysChange"))
              {
                ImGui::OpenPopup("SystemName");
              }
              if (ImGui::BeginPopupModal("SystemName"))
              {
                ImGui::InputText("New Sys Name", newSys, IM_ARRAYSIZE(newSys));

                if (ImGui::Button("Send & Close"))
                {
                  SysName msg = SysName();
                  msg.init();
                  msg.deviceId(deviceId);
                  msg.sysName(newSys);
                  std::cout << "write new SysName";
                  roboClock::control.writeSysName(msg);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                {
                  ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
              }

              // --------------------- BUTTON COMMAND WINDOW -----------------
              if (ImGui::Button("Send ButtonComand"))
              {
                ImGui::OpenPopup("ButtonCommand");
              }
              if (ImGui::BeginPopupModal("ButtonCommand"))
              {
                ImGui::Text("Set values for ButtonCommand");
                // DO THIS STUFFFFFFF TODO
                ImGui::Combo("State", &button_cammand_state_item_number, state_names_list, IM_ARRAYSIZE(state_names_list));
                if (ImGui ::Button("Close and Send"))
                {

                  ButtonCommand comm = ButtonCommand();
                  comm.deviceId(deviceId);
                  std::string color_name = state_names_list[button_cammand_state_item_number];
                  comm.buttonState(state_names_to_color_class[color_name]);
                  roboClock::control.writeToButton(comm);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                  ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
              }
            }
            // ImGui::SetCursorPos(ImVec2(400 - ImGui::CalcTextSize("Close").y, 0));
            ImGui::SetCursorPos(ImVec2(buffer, 400 - ImGui::CalcTextSize("Close").y - buffer));

            if (ImGui::Button("Close This One"))
              ImGui::CloseCurrentPopup();

            ImGui::EndPopup(); // general controll
          }

          ImGui::PopID();
          // add buttons and stuff for the buttons. ( to send sys change message at least.. nick probably too.)

          i++;
        }
      }


      // ---------------------------- CLOCKS -------------------------------
      if (ImGui::CollapsingHeader("CLOCKS"))
      {
        int i = 0;
        for (auto it = org[pair.first].devices[DeviceRole::ROLE_CLOCK].begin(); it != org[pair.first].devices[DeviceRole::ROLE_CLOCK].end(); it++)
        {
          ImGui::Text((*it).c_str());
          std::string deviceId = *it;
          ImGui::PushID(i);
          if (ImGui::Button("Clock Settings"))
          {
            ImGui::OpenPopup("ClockSettings");
          }
          if (ImGui::BeginPopupModal("ClockSettings"))
          {
            // --------------------- Sys Change --------------------------
            if (ImGui::Button("send SysChange"))
              {
                ImGui::OpenPopup("SystemName");
              }
              if (ImGui::BeginPopupModal("SystemName"))
              {
                ImGui::InputText("New Sys Name", newSys, IM_ARRAYSIZE(newSys));

                if (ImGui::Button("Send & Close"))
                {
                  SysName msg = SysName();
                  msg.init();
                  msg.deviceId(deviceId);
                  msg.sysName(newSys);
                  std::cout << "write new SysName";
                  roboClock::control.writeSysName(msg);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                {
                  ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
              }
              // ---------------------- Clock Command --------------------------
              if (ImGui::Button("Send ClockComand"))
              {
                ImGui::OpenPopup("ClockCommand");
              }
              if (ImGui::BeginPopupModal("ClockCommand"))
              {
                ImGui::Text("Set values for ClockCommand");
                // DO THIS STUFFFFFFF TODO
                ImGui::Combo("State", &clock_cammand_state_item_number, state_names_list, IM_ARRAYSIZE(state_names_list)); // this is for color... 
                // need all of ther otehr  things, time, is on, ect.... 
                // 
                // do that here
                if (ImGui ::Button("Close and Send"))
                {

                  ClockCommand comm = ClockCommand();
                  std::string color_name = state_names_list[button_cammand_state_item_number];
                  comm.mainColor(state_names_to_color_class[color_name]);
                  roboClock::control.writeToClock(comm);
                  ImGui::CloseCurrentPopup();
                }
                if (ImGui::Button("Close"))
                  ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
              }

              // -------------------- CLock Dur ------------------------------------
              if(ImGui::Button("Set Clock Duration")){
                ImGui::OpenPopup("SetDuration");
              }
              if(ImGui::BeginPopupModal("SetDuration")){
                ImGui::InputFloat("Clock Duration: (sec) ", &(org[pair.first].durationSec));
              }
            if (ImGui::Button("Close"))
              ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
          }

          ImGui::PopID();

          i++;
        }
      }
      ImGui::Text("\n\n");

      // next : sett systtem duratuiion ?, start, write states, indicatiooon oof buttons ready. reset to state... popup/window on start with pause / sttoop butttons?
      // enable 3 - secoond cooountdown?
      ImGui::Text("Timer Runninig: %d", org[system_name].timer.isRunning());
      if(org[system_name].timer.isRunning()){
        if(org[system_name].state == 4){
          ImGui::Text("Time Remaining: %d", (3 - org[system_name].timer.elapsedMsec()/1000));
        } else {
          ImGui::Text("Time Remaining: %f", (org[system_name].durationSec - org[system_name].timer.elapsedMsec()/1000));
        }
      }
      
      


      // Start Button. -------------------------
      if(org[system_name].state == 3){
        ImGui::PushStyleColor(ImGuiCol_Button, green_col);
      } else {
        ImGui::PushStyleColor(ImGuiCol_Button, grey_col);
      }

      if(ImGui::Button("START", ImVec2(200,100))){
        if(org[system_name].doCountdown){
          roboClock::control.known_devices.setOrgState(system_name, 10); // ?
        }
        else{
          roboClock::control.known_devices.setOrgState(system_name, 11); // ?
        }
      }
      ImGui::PopStyleColor();

      // do countdown: --------------------------
      ImGui::SameLine();
      if(org[system_name].doCountdown){
        // green button, than will make it off. 
        ImGui::PushStyleColor(ImGuiCol_Button, green_col);

        if(ImGui::Button(" [ 3-2-1 ON ] ", ImVec2(200,100))){
          roboClock::control.known_devices.setDoCountdown(system_name, false);

        }
      } else{
        ImGui::PushStyleColor(ImGuiCol_Button, grey_col);

        if(ImGui::Button(" [ 3-2-1 OFF ] ", ImVec2(200,100))){
          roboClock::control.known_devices.setDoCountdown(system_name, true);
        }

      }
      ImGui::PopStyleColor();

      // Declare Winner: 
      if(org[system_name].state == 5 || org[system_name].state == 13){  // may add other ones to this
        // green button, than will make it off. 
        ImGui::PushStyleColor(ImGuiCol_Button, blue_col);

      } else{
        ImGui::PushStyleColor(ImGuiCol_Button, grey_col);

      }
      if(ImGui::Button(" BLUE WIN ", ImVec2(200,100))){
        roboClock::control.known_devices.setOrgState(system_name, 6);
      }
      ImGui::PopStyleColor(1);
      ImGui::SameLine();
      if(org[system_name].state == 5 || org[system_name].state == 13 ){  // may add other ones to this
        // green button, than will make it off. 
        ImGui::PushStyleColor(ImGuiCol_Button, orange_col);

      } else{
        ImGui::PushStyleColor(ImGuiCol_Button, grey_col);

      }
      if(ImGui::Button(" ORANGE WIN ", ImVec2(200,100))){
        roboClock::control.known_devices.setOrgState(system_name, 7);
      }

      ImGui::PopStyleColor();

      //Pause.
      if(org[system_name].timer._running){
        // green button, than will make it off. 
        ImGui::PushStyleColor(ImGuiCol_Button, green_col);

      } else{
        ImGui::PushStyleColor(ImGuiCol_Button, yellow_col);

      }
      if(ImGui::Button(" Pause ", ImVec2(200,100))){
        roboClock::control.known_devices.setOrgState(system_name, 9);
      }
      ImGui::PopStyleColor();

      
      // Panic
      ImGui::SameLine();
      if(org[system_name].state == 8){
        // green button, than will make it off. 
        ImGui::PushStyleColor(ImGuiCol_Button, green_col);

      } else{
        ImGui::PushStyleColor(ImGuiCol_Button, red_col);

      }
      if(ImGui::Button("AHHHHHHHH!!!!!", ImVec2(200,100))){
        roboClock::control.known_devices.setOrgState(system_name, 8);
      }
      ImGui::PopStyleColor();

      // aadd buttons here - start (3-2-1), panic, winner?, unstick? - grey out when not in the state we consider to be "right"


      if( ImGui::Button( "Reset", ImVec2(200,100) )){
        org[system_name].timer.stop();
        roboClock::control.known_devices.setOrgState(system_name, 0);
      }
      ImGui::SameLine();
      if (ImGui::Button("Close", ImVec2(200,100)))
      {
        pair.second = false; // not sure this will stay in the map.
      }
      ImGui::End();
    }
  }


}