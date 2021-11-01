/*
MIT License

Copyright (c) 2010 Gaetan Guidet

This file is part of gcore.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
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
