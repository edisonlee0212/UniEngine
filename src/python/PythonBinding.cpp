//
// Created by lllll on 11/23/2021.
//
#include "Application.hpp"

#include <pybind11/pybind11.h>

using namespace UniEngine;
using namespace pybind11;

int RunApplication()
{
    ApplicationConfigs applicationConfigs;
    Application::Create(applicationConfigs);
    Application::Start();
    return 0;
}

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace py = pybind11;

PYBIND11_MODULE(pyuniengine, m)
{
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------
        .. currentmodule:: cmake_example
        .. autosummary::
           :toctree: _generate
           add
           subtract
    )pbdoc";
    py::class_<Application>(m, "Application").def("Create", [](){
                                                 ApplicationConfigs applicationConfigs;
                                                 Application::Create(applicationConfigs);
                                                 Application::Start();
                                                 Application::End();
                                             });
    m.def(
        "RunApplication",
        []() {
            ApplicationConfigs applicationConfigs;
            Application::Create(applicationConfigs);
            Application::Start();
            Application::End();
        },
        R"pbdoc(
        Run UniEngine.
    )pbdoc");


    #ifdef VERSION_INFO
        m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
    #else
        m.attr("__version__") = "dev";
    #endif
}