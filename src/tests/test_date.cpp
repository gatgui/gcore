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

#include <gcore/date.h>

int main(int, char**) {
  
  gcore::Date now;
  
  // commented out lines do not work on windows...
  
  std::cout << now << std::endl;
  std::cout << now.format("%%A: %A") << " [strftime: " << now.strftime("%A") << "]" << std::endl;
  std::cout << now.format("%%a: %a") << " [strftime: " << now.strftime("%a") << "]" << std::endl;
  std::cout << now.format("%%B: %B") << " [strftime: " << now.strftime("%B") << "]" << std::endl;
  std::cout << now.format("%%b: %b") << " [strftime: " << now.strftime("%b") << "]" << std::endl;
  std::cout << now.format("%%c: %c") << " [strftime: " << now.strftime("%c") << "]"<< std::endl;
  std::cout << now.format("%%d: %d") << " [strftime: " << now.strftime("%d") << "]"<< std::endl;
  std::cout << now.format("%%H: %H") << " [strftime: " << now.strftime("%H") << "]"<< std::endl;
  std::cout << now.format("%%I: %I") << " [strftime: " << now.strftime("%I") << "]"<< std::endl;
  std::cout << now.format("%%j: %j") << " [strftime: " << now.strftime("%j") << "]"<< std::endl;
  std::cout << now.format("%%m: %m") << " [strftime: " << now.strftime("%m") << "]"<< std::endl;
  std::cout << now.format("%%M: %M") << " [strftime: " << now.strftime("%M") << "]"<< std::endl;
  std::cout << now.format("%%p: %p") << " [strftime: " << now.strftime("%p") << "]"<< std::endl;
  std::cout << now.format("%%S: %S") << " [strftime: " << now.strftime("%S") << "]"<< std::endl;
  std::cout << now.format("%%U: %U") << " [strftime: " << now.strftime("%U") << "]"<< std::endl;
  std::cout << now.format("%%w: %w") << " [strftime: " << now.strftime("%w") << "]"<< std::endl;
  std::cout << now.format("%%W: %W") << " [strftime: " << now.strftime("%W") << "]"<< std::endl;
  std::cout << now.format("%%x: %x") << " [strftime: " << now.strftime("%x") << "]"<< std::endl;
  std::cout << now.format("%%X: %X") << " [strftime: " << now.strftime("%X") << "]"<< std::endl;
  std::cout << now.format("%%y: %y") << " [strftime: " << now.strftime("%y") << "]"<< std::endl;
  std::cout << now.format("%%Y: %Y") << " [strftime: " << now.strftime("%Y") << "]"<< std::endl;
  std::cout << now.format("%%z: %z") << " [strftime: " << now.strftime("%z") << "]"<< std::endl;
  std::cout << now.format("%%Z: %Z") << " [strftime: " << now.strftime("%Z") << "]"<< std::endl;

#ifndef _WIN32
  std::cout << now.format("%%C: %C") << " [strftime: " << now.strftime("%C") << "]" << std::endl;
  std::cout << now.format("%%l: %l") << " [strftime: " << now.strftime("%l") << "]" << std::endl;
#endif
  
  std::cout << "Extra formats (not supported by strftime on windows)" << std::endl;
  std::cout << now.format("%%C: %C") << std::endl;
  std::cout << now.format("%%e: %e") << std::endl;
  std::cout << now.format("%%D: %D") << std::endl;
  std::cout << now.format("%%F: %F") << std::endl;
  std::cout << now.format("%%h: %h") << std::endl;
  std::cout << now.format("%%k: %k") << std::endl;
  std::cout << now.format("%%l: %l") << std::endl;
  std::cout << now.format("%%n: %n") << std::endl;
  std::cout << now.format("%%P: %P") << std::endl;
  std::cout << now.format("%%r: %r") << std::endl;
  std::cout << now.format("%%R: %R") << std::endl;
  std::cout << now.format("%%s: %s") << std::endl;
  std::cout << now.format("%%t: %t") << std::endl;
  std::cout << now.format("%%T: %T") << std::endl;
  std::cout << now.format("%%u: %u") << std::endl;
  
  
  gcore::Date othd;
  
  othd.setDayOfMonth(27);
  othd.setHour(16);
  othd.setMinute(12);
  othd.setSecond(48);
  
  std::cout << std::endl;
  std::cout << "Subtract dates:" << std::endl;
  
  gcore::Date diff = othd - now;
  std::cout << "  " << othd << " - " << now << " = " << diff << std::endl;
  
  std::cout << "In Seconds:" << std::endl;
  std::cout << "  now = " << now.get() << std::endl;
  std::cout << "  othd = " << othd.get() << std::endl;
  std::cout << "  diff = " << diff.get() << std::endl;
  std::cout << "  othd - diff = " << (othd.get() - diff.get()) << std::endl;
  
  std::cout << "Adding date and diff:" << std::endl;
  std::cout << "  othd + diff = " << (othd + diff) << std::endl;

  std::cout << "Comparing dates:" << std::endl;  
  std::cout << "  othd - diff == now? " << (((othd - diff) == now) ? "true" : "false") << std::endl;
  
  std::cout << "  othd < now? " << (othd < now ? "true" : "false") << std::endl;
  std::cout << "  othd > now? " << (othd > now ? "true" : "false") << std::endl;
  std::cout << "  othd <= now? " << (othd <= now ? "true" : "false") << std::endl;
  std::cout << "  othd >= now? " << (othd >= now ? "true" : "false") << std::endl;
  
  std::cout << "  othd - diff < now? " << (othd-diff < now ? "true" : "false") << std::endl;
  std::cout << "  othd - diff > now? " << (othd-diff > now ? "true" : "false") << std::endl;
  std::cout << "  othd - diff <= now? " << (othd-diff <= now ? "true" : "false") << std::endl;
  std::cout << "  othd - diff >= now? " << (othd-diff >= now ? "true" : "false") << std::endl;
  
  diff = gcore::Date::Years(2) +
         gcore::Date::Months(1) +
         gcore::Date::Weeks(2) +
         gcore::Date::Days(3) +
         gcore::Date::Hours(5) +
         gcore::Date::Minutes(15) +
         gcore::Date::Seconds(30);
  std::cout << "Subtract: " << diff << std::endl;
  std::cout << "From: " << now << std::endl;
  std::cout << "= " << (now - diff) << std::endl;
  
  return 0;
}
