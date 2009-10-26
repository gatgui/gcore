/*

Copyright (C) 2009  Gaetan Guidet

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

#include <gcore/rexp.h>
using namespace std;
using namespace gcore;

int main(int,char**)
{
	RexError err;
	
	//Regexp  re("(\\d\\d):(\\d\\d)", REX_CAPTURE, &err);
  Regexp    re(IEC("(\d\d):(\d\d)"), REX_CAPTURE, &err);
	MatchData md(3, REX_FORWARD|REX_NOT_EMPTY);
	
	fprintf(stderr, "%s\n", re.getError(err));
	
	string str = "Time -> 10:23";
	
	if (re.match(str, md))
	{
		fprintf(stderr, "%s",               md.getPre(str).c_str() );
		fprintf(stderr, "<<%s>>",           md.get(str,0).c_str()  );
		fprintf(stderr, "%s\n",             md.getPost(str).c_str());
		fprintf(stderr, "Found1: \'%s\'\n", md.get(str,1).c_str()  );
		fprintf(stderr, "Found2: \'%s\'\n", md.get(str,2).c_str()  );
	}
	else
	{
		fprintf(stderr, "Not found\n");
	}
}



