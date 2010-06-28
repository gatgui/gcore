/*

Copyright (C) 2010  Gaetan Guidet

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

#include <gcore/date.h>

int main(int, char**) {
  
  gcore::Date now;
  
  // commented out lines do not work on windows...
  
  std::cout << now << std::endl;
  std::cout << now.format("%%A (%%a): %A (%a)") << " [strftime: " << now.strftime("%A %a") << "]" << std::endl;
  std::cout << now.format("%%B (%%b): %B (%b)") << " [strftime: " << now.strftime("%B %b") << "]"<< std::endl;
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
  
  
  gcore::Date n2;
  
  n2.setDay(27);
  n2.setHour(16);
  n2.setMinute(12);
  n2.setSecond(48);
  
  std::cout << std::endl;
  std::cout << n2 << " - " << now << std::endl;
  
  n2 -= now;
  
  std::cout << "=> " << n2 << std::endl;
  
  return 0;
}
