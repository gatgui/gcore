#include <gcore/log.h>

int main(int, char**)
{
   gcore::Log::EnableColors(true);
   gcore::Log::ShowTimeStamps(true);
   gcore::Log::SetIndentWidth(2);
   gcore::Log::SetIndentLevel(1);
   gcore::Log::SelectOutputs(gcore::LOG_ALL);
   gcore::Log::PrintError("an error");
   gcore::Log::PrintWarning("a warning");
   gcore::Log::PrintDebug("a debug message");
   gcore::Log::PrintInfo("an info message");
   
   return 0;
}
