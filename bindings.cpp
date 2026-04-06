#include "nanobind/nanobind.h"
#include "nanobind/operators.h"
#include "nanobind/stl/string.h"
#include "nanobind/stl/string_view.h"

#include "dpp.hpp"
#include "sqrt.hpp"

namespace nb = nanobind;

template <typename T>
void bind_decimal(nb::module_ &m, char const* name) {
  auto cl = nb::class_<T>(m, name)
    .def_prop_ro_static("eps", [](nb::handle) noexcept { return T::eps; })
    .def_prop_ro_static("max", [](nb::handle) noexcept { return T::max; })
    .def_prop_ro_static("min", [](nb::handle) noexcept { return T::min; })

    .def(nb::init<>())

    .def(nb::init<dpp::nan_t>())
    .def(nb::init<bool>())
    .def(nb::init<std::intmax_t>())
    .def(nb::init<double>())
    .def("__init__", [](T* t, std::string_view const& s) {
        new (t) T(dpp::to_decimal<T>(s));
    })

    .def(nb::init_implicit<dpp::nan_t>())
    .def(nb::init_implicit<bool>())
    .def(nb::init_implicit<std::intmax_t>())
    .def(nb::init_implicit<double>())

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
    .def("__str__", [](T const& a) { return dpp::to_string(a); })
    .def("__repr__", [name](T const& a) {
      return std::string("dpp.", 4) + name + "(\"" + dpp::to_string(a) + "\")";
    })

    .def("__copy__", [](T const& self) noexcept { return T(self); })
    .def("__deepcopy__", [](T const& self, nb::dict) noexcept { return T(self); })

    .def("__getstate__", [](T const& a) { return dpp::to_string(a); })
    .def("__setstate__", [](T& a, std::string_view const& s) noexcept {
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

NB_MODULE(dpp, m) {
  nb::class_<dpp::nan_t>(m, "_NaNType")
    .def(nb::init<>())
    .def("__repr__", [](dpp::nan_t) noexcept { return "nan"; });
  m.attr("nan") = dpp::nan_t();

  bind_decimal<dpp::d32>(m, "d32");
  bind_decimal<dpp::d64>(m, "d64");
  bind_decimal<dpp::d128>(m, "d128");
  bind_decimal<dpp::d256>(m, "d256");
  bind_decimal<dpp::d512>(m, "d512");
}
