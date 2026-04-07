import os
import sysconfig
from setuptools import setup, Extension

# Automatically find the path to nanobind files
site_pkg = sysconfig.get_path("purelib")
nanobind_root = os.path.join(site_pkg, "nanobind")
nb_src = os.path.join(nanobind_root, "src", "nb_combined.cpp")
nb_include = os.path.join(nanobind_root, "include")
nb_robin_map = os.path.join(nanobind_root, "ext", "robin_map", "include")

ext_modules = [
    Extension(
        "dpp",
        ["bindings.cpp", nb_src],
        include_dirs=[nb_include, nb_robin_map],
        language="c++",
        extra_compile_args=[
            "-std=c++20",
            "-O3",
            "-DNDEBUG",
            "-flto",
            "-fno-stack-protector",
            "-fvisibility=hidden",
            "-Wall",
            "-Wextra",
            "-g0"
        ],
        extra_link_args=[
            "-flto",
            "-Wl,--strip-all"
        ]
    )
]

setup(
    name="dpp-dec-float",
    ext_modules=ext_modules
)
