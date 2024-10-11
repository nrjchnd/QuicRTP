cmake_minimum_required(VERSION 3.10)
project(QuicRTP)

set(CMAKE_CXX_STANDARD 17)

# Set /usr/local paths before others
set(CMAKE_PREFIX_PATH "/usr/local" ${CMAKE_PREFIX_PATH})
set(CMAKE_INCLUDE_PATH "/usr/local/include" ${CMAKE_INCLUDE_PATH})
set(CMAKE_LIBRARY_PATH "/usr/local/lib" ${CMAKE_LIBRARY_PATH})

# Add cmake module path
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Add CMake prefix path for hiredis
set(CMAKE_PREFIX_PATH "/usr/local/lib/cmake" ${CMAKE_PREFIX_PATH})
# Find packages
find_package(Boost COMPONENTS system thread REQUIRED)
find_package(fmt REQUIRED)
find_package(redis++ REQUIRED)

# Find hiredis (should be found via redis++)
find_package(hiredis REQUIRED)

# Find SRTP
find_library(SRTP_LIBRARY srtp2 REQUIRED)
find_path(SRTP_INCLUDE_DIR srtp2/srtp.h REQUIRED)

# Find MsQuic
find_library(MSQUIC_LIBRARY msquic REQUIRED)
find_path(MSQUIC_INCLUDE_DIR msquic.h REQUIRED)

# Include directories
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${REDIS++_INCLUDE_DIRS}
    ${HIREDIS_INCLUDE_DIRS}
    ${FMT_INCLUDE_DIRS}
    ${SRTP_INCLUDE_DIR}
    ${MSQUIC_INCLUDE_DIR}
    # Other include paths...
)
# Define installation directories
set(INSTALL_BINDIR bin)
set(INSTALL_LIBDIR lib)
set(INSTALL_INCLUDEDIR include)
set(INSTALL_SYSCONFDIR /etc/quicrtp)

# Source files
set(SOURCES
    main.cpp
    config.cpp
    rtp_listener.cpp
    quic_client.cpp
    translator.cpp
    session_manager.cpp
    cache_manager.cpp
    logger.cpp
)

# Set RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "/usr/local/lib")

# Executable target
add_executable(QuicRtp ${SOURCES})

# Link libraries
target_link_libraries(QuicRtp
    ${Boost_LIBRARIES}
    ${HIREDIS_LIBRARIES}
    redis++::redis++
    fmt::fmt
    ${SRTP_LIBRARY}
    ${MSQUIC_LIBRARY}
    # Other necessary libraries...
)

# Install the executable
install(TARGETS QuicRtp
    RUNTIME DESTINATION ${INSTALL_BINDIR}
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_LIBDIR}
)

# Install configuration file sample
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/quicrtp.conf.sample
               ${CMAKE_CURRENT_BINARY_DIR}/quicrtp.conf.sample
               COPYONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/quicrtp.conf.sample
        DESTINATION ${INSTALL_SYSCONFDIR}
        RENAME quicrtp.conf.sample
)
# Ensure C++17 is used
set_target_properties(QuicRtp PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)
include(InstallRequiredSystemLibraries)
set(CPACK_PACKAGE_NAME "QuicRTP")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
include(CPack)