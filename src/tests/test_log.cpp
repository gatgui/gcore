/*

Copyright (C) 2010~  Gaetan Guidet

This file is part of gcore.

gcore is free software; you can redistribute it and/or modify it
under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at
your option) any later version.

gcore is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
USA.

*/

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
