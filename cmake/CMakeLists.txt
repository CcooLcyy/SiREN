# include the config module for some env set.
include(config.cmake)

# include config vcpkg module, it contain all you want use.
include(vcpkg.cmake)

# include the test module, it is depent on config module when turn on or off.
include(test.cmake)
