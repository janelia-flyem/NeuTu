#include "zsysteminfo.h"

#include "z3dgl.h"
#include "QsLog.h"
#include "z3dgpuinfo.h"
#include "z3dmainwindow.h"
#include <glbinding/Binding.h>
#include <glbinding/Meta.h>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QSettings>
#include <QApplication>
#include <QDateTime>
#include <chrono>

#if !defined(Q_OS_WIN) && !defined(Q_OS_DARWIN)
#include <sys/utsname.h> // for uname
#endif

#ifdef Q_OS_WIN
#include "zwindowsheader.h"
#include <string>
#include <sstream>
using PGNSI = void (WINAPI *)(LPSYSTEM_INFO);
using PGPI = BOOL (WINAPI *)(DWORD, DWORD, DWORD, DWORD, PDWORD);
//#define PRODUCT_PROFESSIONAL	0x00000030   //defined by winnt.h of mingw
#define VER_SUITE_WH_SERVER	0x00008000
#endif

#ifdef Q_OS_DARWIN

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CFBundle.h>

#endif

namespace {

#ifdef Q_OS_WIN
bool windowsVersionName(std::wstring &osString)
{
  OSVERSIONINFOEX osvi;
  SYSTEM_INFO si;
  BOOL bOsVersionInfoEx;
  DWORD dwType;
  ZeroMemory(&si, sizeof(SYSTEM_INFO));
  ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*) &osvi);
  if(!bOsVersionInfoEx)
    return false; // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
  PGNSI pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
  if(pGNSI)
    pGNSI(&si);
  else GetSystemInfo(&si); // Check for unsupported OS
  if (VER_PLATFORM_WIN32_NT != osvi.dwPlatformId || osvi.dwMajorVersion <= 4 ) {
    return false;
  }
  std::wstringstream os;
  os << L"Microsoft "; // Test for the specific product. if ( osvi.dwMajorVersion == 6 )
  {
    if( osvi.dwMinorVersion == 0 ) {
      if( osvi.wProductType == VER_NT_WORKSTATION )
        os << "Windows Vista ";
      else os << "Windows Server 2008 ";
    } else if ( osvi.dwMinorVersion == 1 ) {
      if( osvi.wProductType == VER_NT_WORKSTATION )
        os << "Windows 7 ";
      else os << "Windows Server 2008 R2 ";
    } else if (osvi.dwMinorVersion == 2) {
        if (osvi.wProductType == VER_NT_WORKSTATION)
           os << "Windows 8 ";
        else os << "Windows Server 2012";
    }
    PGPI pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
    pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);
    switch( dwType )
    {
    case PRODUCT_ULTIMATE:
      os << "Ultimate Edition";
      break;
    case PRODUCT_PROFESSIONAL:
      os << "Professional";
      break;
    case PRODUCT_HOME_PREMIUM:
      os << "Home Premium Edition";
      break;
    case PRODUCT_HOME_BASIC:
      os << "Home Basic Edition";
      break;
    case PRODUCT_ENTERPRISE:
      os << "Enterprise Edition";
      break;
    case PRODUCT_BUSINESS:
      os << "Business Edition";
      break;
    case PRODUCT_STARTER:
      os << "Starter Edition";
      break;
    case PRODUCT_CLUSTER_SERVER:
      os << "Cluster Server Edition";
      break;
    case PRODUCT_DATACENTER_SERVER:
      os << "Datacenter Edition";
      break;
    case PRODUCT_DATACENTER_SERVER_CORE:
      os << "Datacenter Edition (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER:
      os << "Enterprise Edition";
      break;
    case PRODUCT_ENTERPRISE_SERVER_CORE:
      os << "Enterprise Edition (core installation)";
      break;
    case PRODUCT_ENTERPRISE_SERVER_IA64:
      os << "Enterprise Edition for Itanium-based Systems";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER:
      os << "Small Business Server";
      break;
    case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
      os << "Small Business Server Premium Edition";
      break;
    case PRODUCT_STANDARD_SERVER:
      os << "Standard Edition";
      break;
    case PRODUCT_STANDARD_SERVER_CORE:
      os << "Standard Edition (core installation)";
      break;
    case PRODUCT_WEB_SERVER:
      os << "Web Server Edition";
      break;
    }
  } if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
  {
    if( GetSystemMetrics(SM_SERVERR2) )
      os <<  "Windows Server 2003 R2, ";
    else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
      os <<  "Windows Storage Server 2003";
    else if ( osvi.wSuiteMask & VER_SUITE_WH_SERVER )
      os <<  "Windows Home Server";
    else if( osvi.wProductType == VER_NT_WORKSTATION &&
             si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
    {
      os <<  "Windows XP Professional x64 Edition";
    }
    else os << "Windows Server 2003, ";  // Test for the server type.
    if ( osvi.wProductType != VER_NT_WORKSTATION )
    {
      if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
      {
        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter Edition for Itanium-based Systems";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise Edition for Itanium-based Systems";
      }   else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
      {
        if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter x64 Edition";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise x64 Edition";
        else os <<  "Standard x64 Edition";
      }   else
      {
        if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
          os <<  "Compute Cluster Edition";
        else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
          os <<  "Datacenter Edition";
        else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
          os <<  "Enterprise Edition";
        else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
          os <<  "Web Edition";
        else os <<  "Standard Edition";
      }
    }
  }
  if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
  {
    os << "Windows XP ";
    if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
      os <<  "Home Edition";
    else os <<  "Professional";
  }
  if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
  {
    os << "Windows 2000 ";  if ( osvi.wProductType == VER_NT_WORKSTATION )
    {
      os <<  "Professional";
    }
    else
    {
      if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
        os <<  "Datacenter Server";
      else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
        os <<  "Advanced Server";
      else os <<  "Server";
    }
  } // Include service pack (if any) and build number.
  if(wcslen(osvi.szCSDVersion) > 0) {
    os << " " << osvi.szCSDVersion;
  }
  os << L" (build " << osvi.dwBuildNumber << L")";
  if ( osvi.dwMajorVersion >= 6 ) {
    if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
      os <<  ", 64-bit";
    else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
      os << ", 32-bit";
  }
  osString = os.str();
  return true;
}
#endif

} // namespace

ZSystemInfo& ZSystemInfo::instance()
{
  static ZSystemInfo ins;
  return ins;
}

ZSystemInfo::ZSystemInfo()
{
  detectOS();

  // shader path
  m_shaderPath = ":/Resources/shader";

  // font path
  m_fontPath = ":/Resources/fonts";
}

void ZSystemInfo::logOSInfo() const
{
  //LOG(INFO) << "OS: " << m_osString;
  LOG(INFO) << "OS: " << QSysInfo::prettyProductName();
  LOG(INFO) << "Kernel: " << QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
  LOG(INFO) << "Build ABI: " << QSysInfo::buildAbi();
  //LOG(INFO) << "Build CPU: " << QSysInfo::buildCpuArchitecture();
  LOG(INFO) << "Current CPU: " << QSysInfo::currentCpuArchitecture();
  LOG(INFO) << "Machine Host Name: " << QSysInfo::machineHostName();
  //LOG(INFO) << "Product Type: " << QSysInfo::productType();
  //LOG(INFO) << "Product Version: " << QSysInfo::productVersion();

#if 1
  // time
  LINFO() << "system_clock res: "
          << 1e9 * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den << " ns";
  LINFO() << "system_clock is_steady = " << std::boolalpha << std::chrono::system_clock::is_steady;

  LINFO() << "steady_clock res: "
          << 1e9 * std::chrono::steady_clock::period::num / std::chrono::steady_clock::period::den << " ns";
  LINFO() << "steady_clock is_steady = " << std::boolalpha << std::chrono::steady_clock::is_steady;

  LINFO() << "high_resolution_clock res: "
          << 1e9 * std::chrono::high_resolution_clock::period::num / std::chrono::high_resolution_clock::period::den
          << " ns";
  LINFO() << "high_resolution_clock is_steady = " << std::boolalpha << std::chrono::high_resolution_clock::is_steady;
#endif
}

bool ZSystemInfo::initializeGL()
{
  if (m_glInitialized) {
    LOG(ERROR) << "OpenGL already initialized. Skip.";
    return false;
  }

  glbinding::Binding::initialize();
  Z3DGpuInfo::instance().logGpuInfo();
#if defined(CHECK_OPENGL_ERROR_FOR_ALL_GL_CALLS)
  glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After |
                                   glbinding::CallbackMask::ParametersAndReturnValue |
                                   glbinding::CallbackMask::Unresolved,
                                   {"glGetError"});
  glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      std::ostringstream os;

      os << call.function->name() << "(";
      for (size_t i = 0; i < call.parameters.size(); ++i) {
        os << call.parameters[i]->asString();
        if (i + 1 < call.parameters.size())
          os << ", ";
      }
      os << ")";

      if (call.returnValue) {
        os << " -> " << call.returnValue->asString();
      }

      LOG(ERROR) << "OpenGL error: " << glbinding::Meta::getString(error) << " with " << os.str();
    }
  });
#elif 0
  glbinding::setCallbackMaskExcept(glbinding::CallbackMask::After |
                                   glbinding::CallbackMask::ParametersAndReturnValue |
                                   glbinding::CallbackMask::Unresolved,
                                   {"glGetError"});
  glbinding::setAfterCallback([](const glbinding::FunctionCall& call) {
    std::cout << call.function->name() << "(";

    for (size_t i = 0; i < call.parameters.size(); ++i) {
      std::cout << call.parameters[i]->asString();
      if (i < call.parameters.size() - 1)
        std::cout << ", ";
    }

    std::cout << ")";

    if (call.returnValue) {
      std::cout << " -> " << call.returnValue->asString();
    }

    std::cout << "\n";

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
      std::cout << "OpenGL error: " << glbinding::Meta::getString(error) << "\n";

    std::cout.flush();
  });
#else
  glbinding::setCallbackMask(glbinding::CallbackMask::Unresolved);
#endif
  glbinding::setUnresolvedCallback([](const glbinding::AbstractFunction& call) {
    LOG(ERROR) << "OpenGL function " << call.name() << " can not be resolved.";
  });
  glbinding::Binding::addContextSwitchCallback([](glbinding::ContextHandle handle) {
    LOG(INFO) << "Switching to OpenGL context " << handle;
  });
  if (Z3DGpuInfo::instance().isSupported()) {
    m_glInitialized = true;
    return m_glInitialized;
  }
  m_errorMsg = Z3DGpuInfo::instance().notSupportedReason();
  LOG(ERROR) << m_errorMsg;
  m_glInitialized = false;
  return m_glInitialized;
}

QString ZSystemInfo::shaderPath(const QString& filename) const
{
  return m_shaderPath + (filename.isEmpty() ? QString("") : QString("/") + filename);
}

QString ZSystemInfo::fontPath(const QString& filename) const
{
  return m_fontPath + (filename.isEmpty() ? QString("") : QString("/") + filename);
}

void ZSystemInfo::detectOS()
{
#ifdef Q_OS_WIN
  std::wstring osString;
  if (windowsVersionName(osString)) {
    m_osString = QString::fromStdWString(osString);
  } else {
    switch (QSysInfo::WindowsVersion) {
    case QSysInfo::WV_NT:
      m_osString = "Windows NT (operating system version 4.0)";
      break;
    case QSysInfo::WV_2000:
      m_osString = "Windows 2000 (operating system version 5.0)";
      break;
    case QSysInfo::WV_XP:
      m_osString = "Windows XP (operating system version 5.1)";
      break;
    case QSysInfo::WV_2003:
      m_osString = "Windows Server 2003, Windows Server 2003 R2, Windows Home Server, "
                   "Windows XP Professional x64 Edition (operating system version 5.2)";
      break;
    case QSysInfo::WV_VISTA:
      m_osString = "Windows Vista, Windows Server 2008 (operating system version 6.0)";
      break;
    case QSysInfo::WV_WINDOWS7:
      m_osString = "Windows 7, Windows Server 2008 R2 (operating system version 6.1)";
      break;
    case QSysInfo::WV_WINDOWS8:
      m_osString = "Windows 8 (operating system version 6.2)";
      break;
    case QSysInfo::WV_WINDOWS8_1:
      m_osString = "Windows 8.1 (operating system version 6.3)";
      break;
    default:
      m_osString = "unknown win os";
    }
  }
#elif defined(Q_OS_DARWIN)
  switch (QSysInfo::MacintoshVersion) {
    case QSysInfo::MV_10_7:
      m_osString = "Mac OS X LION";
      break;
    case QSysInfo::MV_10_8:
      m_osString = "Mac OS X MOUNTAIN LION";
      break;
    case QSysInfo::MV_10_9:
      m_osString = "Mac OS X MAVERICKS";
      break;
    case QSysInfo::MV_10_10:
      m_osString = "Mac OS X YOSEMITE";
      break;
    case QSysInfo::MV_10_11:
      m_osString = "Mac OS X El Capitan";
      break;
    case QSysInfo::MV_10_12:
      m_osString = "macOS Sierra";
      break;
    default:
      m_osString = "unknown mac os";
      return;
  }
  // deprecated from 10.8
  //SInt32 majorVersion,minorVersion,bugFixVersion;
  //Gestalt(gestaltSystemVersionMajor, &majorVersion);
  //Gestalt(gestaltSystemVersionMinor, &minorVersion);
  //Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
  //m_osString += QString(" %1.%2.%3").arg(majorVersion).arg(minorVersion).arg(bugFixVersion);
#else
  utsname name;
  if (uname(&name) != 0)
    return; // command not successful

  m_osString = QString("%1 %2 %3 %4").arg(name.sysname).arg(name.release).arg(name.version).arg(name.machine);

#endif // Q_OS_WIN
}
