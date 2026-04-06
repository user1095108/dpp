#include "nanobind/nanobind.h"
#include "nanobind/operators.h"
#include "nanobind/stl/string.h"
#include "nanobind/stl/string_view.h"

#include "dpp.hpp"
#include "sqrt.hpp"

namespace nb = nanobind;

template <typename T>
void bind_decimal(nb::module_ &m, char const* name) {
  nb::class_<T>(m, name)
    // Constructors
    .def(nb::init<>())
    .def(nb::init<dpp::nan_t>())
    .def(nb::init<bool>())
    .def(nb::init<double>())
    .def(nb::init<std::intmax_t>())
    .def("__init__", [](T* t, std::string_view const& s) {
        new (t) T(dpp::to_decimal<T>(s));
    })

    // 1. Standard Arithmetic (+, -, *, /)
    .def(nb::self + nb::self)
    .def(nb::self + std::intmax_t()).def(std::intmax_t() + nb::self)
    .def(nb::self + double()).def(double() + nb::self)

    .def(nb::self - nb::self)
    .def(nb::self - std::intmax_t()).def(std::intmax_t() - nb::self)
    .def(nb::self - double()).def(double() - nb::self)

    .def(nb::self * nb::self)
    .def(nb::self * std::intmax_t()).def(std::intmax_t() * nb::self)
    .def(nb::self * double()).def(double() * nb::self)

    .def(nb::self / nb::self)
    .def(nb::self / std::intmax_t()).def(std::intmax_t() / nb::self)
    .def(nb::self / double()).def(double() / nb::self)

    // 2. Augmented Assignment (+=, -=, *=, /=)
    .def(nb::self += nb::self).def(nb::self += std::intmax_t()).def(nb::self += double())
    .def(nb::self -= nb::self).def(nb::self -= std::intmax_t()).def(nb::self -= double())
    .def(nb::self *= nb::self).def(nb::self *= std::intmax_t()).def(nb::self *= double())
    .def(nb::self /= nb::self).def(nb::self /= std::intmax_t()).def(nb::self /= double())

    // 3. Unary Operators
    .def(-nb::self)
    .def(+nb::self)

    // 4. Comparisons (==, !=, <, <=, >, >=)
    .def(nb::self == dpp::nan_t()).def(dpp::nan_t() == nb::self)
    .def(nb::self != dpp::nan_t()) .def(dpp::nan_t() != nb::self)

    .def(nb::self == nb::self)
    .def(nb::self == std::intmax_t()).def(std::intmax_t() == nb::self)
    .def(nb::self == double()).def(double() == nb::self)

    .def(nb::self != nb::self)
    .def(nb::self != std::intmax_t()).def(std::intmax_t() != nb::self)
    .def(nb::self != double()).def(double() != nb::self)

    .def(nb::self < nb::self)
    .def(nb::self < std::intmax_t()).def(std::intmax_t() < nb::self)
    .def(nb::self < double()).def(double() < nb::self)

    .def(nb::self <= nb::self)
    .def(nb::self <= std::intmax_t()).def(std::intmax_t() <= nb::self)
    .def(nb::self <= double()).def(double() <= nb::self)

    .def(nb::self > nb::self)
    .def(nb::self > std::intmax_t()).def(std::intmax_t() > nb::self)
    .def(nb::self > double()).def(double() > nb::self)

    .def(nb::self >= nb::self)
    .def(nb::self >= std::intmax_t()).def(std::intmax_t() >= nb::self)
    .def(nb::self >= double()).def(double() >= nb::self)

    // 5. Conversions & Protocols
    .def("__copy__", [](T const& self) noexcept { return T(self); })
    .def("__deepcopy__", [](T const& self, nb::dict) noexcept { return T(self); })
    .def("__float__", [](T const& a) noexcept { return double(a); })
    .def("__int__", [](T const& a) { return std::intmax_t(a); })
    .def("__hash__", [](T const& a) noexcept { return std::hash<T>{}(a); })
    .def("__str__", [](T const& a) { return dpp::to_string(a); })
    .def("__repr__", [name](T const& a) {
      return std::string("dpp.") + name + "(\"" + dpp::to_string(a) + "\")";
    })
    .def("__getstate__", [](T const& a) { return dpp::to_string(a); })
    .def("__setstate__", [](T& a, std::string_view const& s) {
      new (&a) T(dpp::to_decimal<T>(s));
    });
}

NB_MODULE(dpp, m) {
  nb::class_<dpp::nan_t>(m, "_NaNType")
    .def(nb::init<>())
    .def("__repr__", [](dpp::nan_t const&) { return "nan"; });
  m.attr("nan") = dpp::nan_t();

  bind_decimal<dpp::d32>(m, "d32");
  bind_decimal<dpp::d64>(m, "d64");
  bind_decimal<dpp::d128>(m, "d128");
  bind_decimal<dpp::d256>(m, "d256");
  bind_decimal<dpp::d512>(m, "d512");

  m.def("sqrt", &dpp::sqrt<dpp::d32::sig_t, dpp::d32::exp_t>);
  m.def("sqrt", &dpp::sqrt<dpp::d64::sig_t, dpp::d64::exp_t>);
  m.def("sqrt", &dpp::sqrt<dpp::d128::sig_t, dpp::d128::exp_t>);
  m.def("sqrt", &dpp::sqrt<dpp::d256::sig_t, dpp::d256::exp_t>);
  m.def("sqrt", &dpp::sqrt<dpp::d512::sig_t, dpp::d512::exp_t>);
}
