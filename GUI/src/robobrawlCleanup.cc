#include "robobrawlClock.hh"

void
roboClock::cleanup( )
{
  std::cout << "CLEASNUPPPPP";
  gui::close_renderer();
  gui::close_backend();
  gui::close_imgui(); // I think this is the right orde .
  control.clear();
  context.clear();
  

}
