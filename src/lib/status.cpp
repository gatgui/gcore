#include <gcore/status.h>
#include <gcore/platform.h>

namespace gcore {

Status::Status() {
  set(true);
}

Status::Status(bool success) {
  set(success);
}

Status::Status(bool success, int errcode) {
  set(success, errcode);
}

Status::Status(bool success, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  set(success, msg, args);
  va_end(args);
}

Status::Status(bool success, int errcode, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  set(success, errcode, msg, args);
  va_end(args);
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
  set(true);
}

void Status::set(bool success) {
  mSuccess = success;
  mErrCode = -1;
  mMsg = "";
}

void Status::set(bool success, int errcode) {
  set(success);
  mErrCode = errcode;
  if (!mSuccess) {
    mMsg += " (";
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

void Status::set(bool success, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  set(success, msg, args);
  va_end(args);
}

void Status::set(bool success, int errcode, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  set(success, errcode, msg, args);
  va_end(args);
}

void Status::set(bool success, const char *msg, va_list args) {
  set(success);
  if (msg) {
    char buffer[4096];
    vsnprintf(buffer, 4095, msg, args);
    mMsg = buffer;
  }
}

void Status::set(bool success, int errcode, const char *msg, va_list args) {
  set(success, errcode);
  if (msg) {
    char buffer[4096];
    vsnprintf(buffer, 4095, msg, args);
    mMsg.insert(0, buffer);
  }
}

}
