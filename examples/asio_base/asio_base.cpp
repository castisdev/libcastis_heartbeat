#include "HeartBeatSession.h"

#include <boost/asio.hpp>

int main(int /*argc*/, char* /*argv*/[])
{
  boost::asio::io_service ios;

  HeartBeatSession hbs(ios, "127.0.0.1", "127.0.0.1", 8090);
  hbs.start();


  ios.run();

  return 0;
}
