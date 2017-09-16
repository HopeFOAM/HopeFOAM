#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
// We're on Windows using the Microsoft VC++ 6.0 compiler. We need to 
// include the .h versions of iostream and fstream.

#include <iostream.h>
#include <fstream.h>
#include <strstrea.h>

#else
// Include iostream and some using statements.
#include <iostream>
#include <fstream>
#endif
int main()
{
  std::ofstream out;
  out.open("test", std::ios::out);  
  out.setf(std::ios::unitbuf);
  out.rdbuf()->setbuf((char*)0,0);
  return 0;
}  