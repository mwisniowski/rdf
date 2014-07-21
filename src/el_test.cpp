#include <unistd.h>
#include "helper/easylogging++.h"

#undef _ELPP_HANDLE_SIGABRT
_INITIALIZE_EASYLOGGINGPP
#undef _ELPP_HANDLE_SIGABRT
int main(int argc, char *argv[])
{
  _START_EASYLOGGINGPP( argc, argv );
  el::Loggers::addFlag( el::LoggingFlag::DisableApplicationAbortOnFatalLog );
  el::Loggers::addFlag( el::LoggingFlag::ColoredTerminalOutput );
  el::Loggers::addFlag( el::LoggingFlag::AutoSpacing );

  int i = 1;
  while( i )
  {
    LOG(INFO) << "Logging" << i++;
    sleep( 5 );
  }
  return 0;
}
