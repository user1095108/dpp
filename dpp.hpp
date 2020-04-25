#ifndef DPP_HPP
# define DPP_HPP
# pragma once

#include <cstdint>

#include <type_traits>

namespace
{

template <typename T>
constexpr auto pow2(T const e) noexcept
{
  return T(1) << e;
}

}

template <unsigned M, unsigned E>
class dpp
{
  using value_type = std::conditional_t<M + E <= 16, std::int16_t,
    std::conditional_t<M + E <= 32, std::int32_t,
      std::conditional_t<M + E <= 64, std::int64_t, void>
    >
  >;

  struct
  {
    value_type m:M;
    value_type e:E;
  } v_;

  constexpr void normalize() noexcept
  {
    if (!is_nan() && v_.m)
    {
      for (; !(v_.m % 10); v_.m /= 10)
      {
        ++v_.e;
      }
    }
  }

public:
  constexpr dpp() noexcept
  {
    v_.m = {};
    v_.e = {};
  }

  template <typename U,
    std::enable_if_t<std::is_integral_v<std::decay_t<U>>, int> = 0
  >
  constexpr dpp(U&& v) noexcept
  {
    v_.m = v;
    v_.e = {};

    normalize();
  }

  constexpr dpp(value_type const m, value_type const e) noexcept
  {
    v_.m = m;
    v_.e = e;

    normalize();
  }

  struct nan_{};

  constexpr dpp(nan_&&) noexcept
  {
    v_.m = {};
    v_.e = -pow2(E - 1);
  }

  constexpr auto mantissa() const noexcept
  {
    return v_.m;
  }

  constexpr auto exponent() const noexcept
  {
    return v_.e;
  }

  constexpr bool is_nan() const noexcept
  {
    return v_.e == -pow2(E - 1);
  }

  explicit operator bool() noexcept
  {
    return is_nan() || v_.m;
  }

  //
  constexpr auto operator==(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return false;
    }
    else
    {
      return (v_.m == o.v_.m) && (v_.e == o.v_.e);
    }
  }

  constexpr auto operator!=(dpp const& o) noexcept
  {
    return !operator==(o);
  }

  //
  constexpr auto operator+(dpp const& o) noexcept
  {
    constexpr auto op([](dpp tmp, dpp const& o) noexcept
      {
        while (tmp.v_.e != o.v_.e)
        {
          tmp.v_.m *= 10;
          --tmp.v_.e;
        }

        tmp.v_.m += o.v_.m;

        return tmp;
      }
    );

    if (is_nan() || o.is_nan())
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp;

      if (o.v_.e > v_.e)
      {
        tmp = op(o, *this);
      }
      else if (v_.e > o.v_.e)
      {
        tmp = op(*this, o);
      }
      else
      {
        tmp = *this;

        tmp.v_.m += o.v_.m;
      }

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator+=(dpp const& o) noexcept
  {
    constexpr auto op([](dpp tmp, dpp const& o) noexcept
      {
        while (tmp.v_.e != o.v_.e)
        {
          tmp.v_.m *= 10;
          --tmp.v_.e;
        }

        tmp.v_.m += o.v_.m;

        return tmp;
      }
    );

    if (is_nan() || o.is_nan())
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp;

      if (o.v_.e > v_.e)
      {
        tmp = op(o, *this);
      }
      else if (v_.e > o.v_.e)
      {
        tmp = op(*this, o);
      }
      else
      {
        v_.m += o.v_.m;

        normalize();

        return *this;
      }

      tmp.normalize();

      return *this = tmp;
    }
  }

  //
  constexpr auto operator-(dpp const& o) noexcept
  {
    constexpr auto op([](dpp tmp, dpp const& o) noexcept
      {
        while (tmp.v_.e != o.v_.e)
        {
          tmp.v_.m *= 10;
          --tmp.v_.e;
        }

        tmp.v_.m -= o.v_.m;

        return tmp;
      }
    );

    if (is_nan() || o.is_nan())
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp;

      if (o.v_.e > v_.e)
      {
        tmp = op(o, *this);
      }
      else if (v_.e > o.v_.e)
      {
        tmp = op(*this, o);
      }
      else
      {
        tmp = *this;

        tmp.v_.m -= o.v_.m;
      }

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator-=(dpp const& o) noexcept
  {
    constexpr auto op([](dpp tmp, dpp const& o) noexcept
      {
        while (tmp.v_.e != o.v_.e)
        {
          tmp.v_.m *= 10;
          --tmp.v_.e;
        }

        tmp.v_.m -= o.v_.m;

        return tmp;
      }
    );

    if (is_nan() || o.is_nan())
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp;

      if (o.v_.e > v_.e)
      {
        tmp = op(o, *this);
      }
      else if (v_.e > o.v_.e)
      {
        tmp = op(*this, o);
      }
      else
      {
        v_.m -= o.v_.m;

        normalize();

        return *this;
      }

      tmp.normalize();

      return *this = tmp;
    }
  }

  //
  constexpr auto operator*(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp(*this);

      tmp.v_.m *= o.v_.m;
      tmp.v_.e += o.v_.e;

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator*=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan())
    {
      *this = dpp{nan_{}};
    }
    else
    {
      v_.m *= o.v_.m;
      v_.e += o.v_.e;

      normalize();
    }

    return *this;
  }

  //
  constexpr auto operator/(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan() || !o.v_.m)
    {
      return dpp{nan_{}};
    }
    else
    {
      dpp tmp(*this);

      tmp.v_.m /= o.v_.m;
      tmp.v_.e -= o.v_.e;

      tmp.normalize();

      return tmp;
    }
  }

  constexpr auto& operator/=(dpp const& o) noexcept
  {
    if (is_nan() || o.is_nan() || !o.v_.m)
    {
      *this = dpp{nan_{}};
    }
    else
    {
      v_.m /= o.v_.m;
      v_.e -= o.v_.e;

      normalize();
    }

    return *this;
  }
};

using dec64 = dpp<56, 8>;
using dec32 = dpp<26, 6>;
using dec16 = dpp<12, 4>;

#endif // DPP_HPP
