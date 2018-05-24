#include "zmessagereporter.h"

#include <iostream>
#include <ctime>

using namespace std;

int ZMessageReporter::m_count = 1;

ZMessageReporter::ZMessageReporter()
{
}

void ZMessageReporter::report(
    const std::string &title, const std::string &message,
    neutube::EMessageType msgType)
{
  if (msgType == neutube::MSG_ERROR) {
    report(cerr, title, message, msgType);
  } else {
    report(cout, title, message, msgType);
  }
  /*
  switch (msgType) {
  case INFORMATION:
    cout << title << endl << "  " << message << endl;
    break;
  case WARNING:
    cout  << title << endl << "  " << message << endl;
    break;
  case ERROR:
    cerr << "Error: " << title << endl << "  " << message << endl;
    break;
  case DEBUG:
    cout << "Debug: " << title << endl << "  " << message << endl;
    break;
  }
  */
}

void ZMessageReporter::report(std::ostream &stream,
                              const std::string &title,
                              const std::string &message,
                              neutube::EMessageType msgType)
{
  time_t timer;
  time(&timer);
  stream << "========" << asctime(localtime(&timer));
  stream << "#" << m_count << ": ";

  switch (msgType) {
  case neutube::MSG_INFORMATION:
    stream << title << endl << "  " << message << endl;
    break;
  case neutube::MSG_WARNING:
    stream  << title << endl << "  " << message << endl;
    break;
  case neutube::MSG_ERROR:
    stream << "Error: " << title << endl << "  " << message << endl;
    break;
  case neutube::MSG_DEBUG:
    stream << "Debug: " << title << endl << "  " << message << endl;
    break;
  }

  ++m_count;
}
