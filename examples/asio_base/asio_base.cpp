#include "HeartBeatSession.h"

#include <iostream>
#include <boost/asio.hpp>

int main(int /*argc*/, char* /*argv*/[])
{
  boost::asio::io_service ios;

  try {
    HeartBeatSession hbs(ios, "127.0.0.1", "127.0.0.1", 8090);
    hbs.start();

    ios.run();
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }

  return 0;
}
