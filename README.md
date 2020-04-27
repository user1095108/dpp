# dpp
Not so long ago, computer users were expected to write their own floating point libraries. Various floating point routines were floating around, snippets out of which users could compose their own floating point libraries. These, of course, were not standardized and often unstable. I wrote this library for my personal use. If you decide to use it yourself, be cautious.

# example
```c++
int main()
{
  auto const a(dpp::to_decimal<dpp::dec32>("1.23"));
  auto const b(dpp::to_decimal<dpp::dec32>("45.6"));

  std::cout << a + b << std::endl;
  std::cout << a - b << std::endl;
  std::cout << a * b << std::endl;
  std::cout << a / b << std::endl;
  std::cout << b / a << std::endl;

  return 0;
}
```
