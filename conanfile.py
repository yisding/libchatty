from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout


class LibchattyConan(ConanFile):
    name = "libchatty"
    version = "0.1"
    description = "LLM wrapper for ricers"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    
    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "*.c", "*.h"
    
    def requirements(self):
        self.requires("libcurl/8.10.1")
    
    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.generate()
    
    def build(self):
        from conan.tools.cmake import CMake
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
    
    def package(self):
        from conan.tools.cmake import CMake
        cmake = CMake(self)
        cmake.install()
    
    def package_info(self):
        self.cpp_info.libs = ["chatty"]
        self.cpp_info.includedirs = ["include"]
