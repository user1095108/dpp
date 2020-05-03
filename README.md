# dpp
This is a sample decimal floating point library for testing purposes. The library does not adhere to any particular standard. My goal was to write an easily understandable library, that anyone could fix. For quick decimal jobs.
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
