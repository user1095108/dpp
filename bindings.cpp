#include "nanobind/nanobind.h"
#include "nanobind/operators.h"
#include "nanobind/stl/string.h"
#include "nanobind/stl/string_view.h"

#include <format>
#include <iostream>
#include <memory>
#include <type_traits>

#include "sqrt.hpp"

namespace nb = nanobind;

#define MOD_NAME dpp
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

template <typename T>
void bind_decimal(nb::module_ &m, auto const& name) {
  auto cl = nb::class_<T>(m, name, nb::is_final())
    .def_prop_ro_static("eps", [](nb::handle) noexcept { return T::eps; })
    .def_prop_ro_static("max", [](nb::handle) noexcept { return T::max; })
    .def_prop_ro_static("min", [](nb::handle) noexcept { return T::min; })

    .def(nb::init<>())

    .def(nb::init<dpp::nan_t>())
    .def(nb::init<bool>())
    .def(nb::init<std::intmax_t>())
    .def(nb::init<double>())
    .def("__init__", [](T* const t, std::string_view const& s) {
      static_assert(std::is_trivially_default_constructible_v<T>);
      *t = dpp::to_decimal<T>(s);
    })

    .def(nb::init_implicit<dpp::nan_t>())
    .def(nb::init_implicit<bool>())
    .def(nb::init_implicit<std::intmax_t>())
    .def(nb::init_implicit<double>())

    .def("sig", &T::sig)
    .def("exp", &T::exp)

    .def(nb::self + nb::self)
    .def(nb::self - nb::self)
    .def(nb::self * nb::self)
    .def(nb::self / nb::self)

    .def(-nb::self)
    .def(+nb::self)

    .def(nb::self == dpp::nan_t()).def(dpp::nan_t() == nb::self)
    .def(nb::self != dpp::nan_t()).def(dpp::nan_t() != nb::self)

    .def(nb::self == nb::self)
    .def(nb::self != nb::self)
    .def(nb::self < nb::self)
    .def(nb::self <= nb::self)
    .def(nb::self > nb::self)
    .def(nb::self >= nb::self)

    .def("__bool__", [](T const& a) noexcept { return bool(a); })
    .def("__float__", [](T const& a) noexcept { return double(a); })
    .def("__int__", [](T const& a) noexcept { return std::intmax_t(a); })
    .def("__hash__", [](T const& a) noexcept { return std::hash<T>{}(a); })

    .def("__repr__", [&name](T const& a) {
      return std::format(STR(MOD_NAME)".{}(\"{}\")", name, dpp::to_string(a));
    })
    .def("__str__", [](T const& a) { return dpp::to_string(a); })

    .def("__copy__", [](T const& self) noexcept { return T(self); })
    .def("__deepcopy__", [](T const& self, nb::dict const&) noexcept {
      return T(self);
    })

    .def("__getstate__", [](T const& a) { return dpp::to_string(a); })
    .def("__setstate__", [](T& a, std::string_view const& s) noexcept {
      static_assert(std::is_trivially_default_constructible_v<T>);
      a = dpp::to_decimal<T>(s);
    });

  m.def("isnan", dpp::isnan<typename T::sig_t, typename T::exp_t>);

  m.def("abs", dpp::abs<typename T::sig_t, typename T::exp_t>);
  m.def("trunc", dpp::trunc<typename T::sig_t, typename T::exp_t>);
  m.def("floor", dpp::floor<typename T::sig_t, typename T::exp_t>);
  m.def("ceil", dpp::ceil<typename T::sig_t, typename T::exp_t>);
  m.def("round", dpp::round<typename T::sig_t, typename T::exp_t>);

  m.def("inv", dpp::inv<typename T::sig_t, typename T::exp_t>);

  m.def("fma", dpp::fma<typename T::sig_t, typename T::exp_t>);
  m.def("midpoint", dpp::midpoint<typename T::sig_t, typename T::exp_t>);

  m.def("sqrt", dpp::sqrt<typename T::sig_t, typename T::exp_t>);
}

NB_MODULE(MOD_NAME, m) {
  {
    auto const& name = "nan_t";

    nb::class_<dpp::nan_t>(m, name)
      .def(nb::init<>())
      .def("__repr__", [&name](dpp::nan_t) {
        return std::format(STR(MOD_NAME)".{}()", name);
      });
    m.attr("nan") = dpp::nan_t();
  }

  bind_decimal<dpp::d32>(m, "d32");
  bind_decimal<dpp::d64>(m, "d64");
  bind_decimal<dpp::d128>(m, "d128");
}
