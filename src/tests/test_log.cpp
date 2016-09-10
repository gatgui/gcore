#include <gcore/log.h>

void PrintErr(const char *msg)
{
   std::cerr << msg;
}

int main(int, char**)
{
   gcore::Log::OutputFunc func;
   
   gcore::Bind(PrintErr, func);
   
   gcore::Log::SetOutputFunc(func);
   gcore::Log::SetColorOutput(true);
   gcore::Log::SetShowTimeStamps(true);
   gcore::Log::SetIndentWidth(2);
   gcore::Log::SetIndentLevel(1);
   gcore::Log::SetLevelMask(gcore::LOG_ALL);
   gcore::Log::PrintError("an error");
   gcore::Log::PrintWarning("a warning");
   gcore::Log::PrintDebug("a debug message");
   gcore::Log::PrintInfo("an info message");
   
   return 0;
}
