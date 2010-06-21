#include <iostream>
#include <cstdlib>

int main(int, char **)
{
  char *val = getenv("CUSTOM_ENV");
  std::cout << "Writing stuffs to stdout (CUSTOM_ENV = " << (val ? val : "<undefined>") << ")" << std::endl;
  std::cerr << "Writing stuffs to stderr" << std::endl;
  
  return 0;
}
  