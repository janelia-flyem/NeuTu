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
    neutu::EMessageType msgType)
{
  if (msgType == neutu::EMessageType::ERROR) {
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
                              neutu::EMessageType msgType)
{
  time_t timer;
  time(&timer);
  stream << "========" << asctime(localtime(&timer));
  stream << "#" << m_count << ": ";

  switch (msgType) {
  case neutu::EMessageType::INFORMATION:
    stream << title << endl << "  " << message << endl;
    break;
  case neutu::EMessageType::WARNING:
    stream  << title << endl << "  " << message << endl;
    break;
  case neutu::EMessageType::ERROR:
    stream << "Error: " << title << endl << "  " << message << endl;
    break;
  case neutu::EMessageType::DEBUG:
    stream << "Debug: " << title << endl << "  " << message << endl;
    break;
  }

  ++m_count;
}
