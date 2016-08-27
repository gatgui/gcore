#include <gcore/status.h>

namespace gcore {

Status::Status()
  : mSuccess(true)
  , mErrCode(-1)
  , mMsg("") {
}

Status::Status(bool success, const char *msg) {
  set(success, msg);
}

Status::Status(bool success, int errcode, const char *msg) {
  set(success, errcode, msg);
}

Status::~Status() {
}

Status& Status::operator=(const Status &rhs) {
  mSuccess = rhs.mSuccess;
  mErrCode = rhs.mErrCode;
  mMsg = rhs.mMsg;
  return *this;
}

void Status::clear() {
  mSuccess = true;
  mErrCode = -1;
  mMsg = "";
}

void Status::set(bool success, const char *msg) {
  mSuccess = success;
  mErrCode = -1;
  mMsg = (msg != NULL ? msg : "");
}

void Status::set(bool success, int errcode, const char *msg) {
  mSuccess = success;
  mErrCode = -1;
  mMsg = (msg != NULL ? msg : "");
  if (!mSuccess) {
    mMsg += " (";
    mErrCode = errcode;
#ifdef _WIN32
    LPTSTR buffer = NULL; 
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL, mErrCode, MAKELANGID(LANG_ENGLISH,SUBLANG_DEFAULT),
                  (LPTSTR)&buffer, 0, NULL);
    if (buffer) {
      mMsg += std::string(buffer);
      LocalFree(buffer);
    }
#else
    mMsg += strerror(mErrCode);
#endif
    mMsg += ")";
  }
}

}
