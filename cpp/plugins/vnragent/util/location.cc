// location.cc
// 5/3/2014 jichi
//
// As QDesktopServices is in QtGui, calculate locations myself.
#include "util/location.h"
#include <windows.h>
#include <shlobj.h>

namespace { // unnamed

QString getDesktopLocation()
{
  // Manually get desktop location since QtGui does not exist
  // http://stackoverflow.com/questions/17933917/get-the-users-desktop-folder-using-windows-api
  wchar_t buf[MAX_PATH];
  if (S_OK == ::SHGetFolderPathW(nullptr, CSIDL_DESKTOPDIRECTORY, nullptr, 0, buf))
    return QString::fromWCharArray(buf);
  return QString();
}

} // unnamed namespace

QString Util::desktopLocation()
{
  static QString ret;
  if (ret.isEmpty())
    ret = ::getDesktopLocation();
  return ret;
}
// EOF
