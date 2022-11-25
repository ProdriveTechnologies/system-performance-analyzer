#include <iostream>

void print_func() { std::cout << "Printing the world" << std::endl; }

int main()
{
  for (int i = 0; i < 8; ++i)
  {
    std::cout << "This is the main function" << std::endl;
  }
  print_func();
}
