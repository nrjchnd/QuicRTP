cmake_minimum_required(VERSION 3.10)
project(QuicRtpTranslator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(/usr/local/include)

# Link directories
link_directories(/usr/local/lib)

# Source files
set(SOURCES
    main.cpp
    config.cpp
    rtp_listener.cpp
    quic_client.cpp
    translator.cpp
    session_manager.cpp
    cache_manager.cpp
)

# Add executable
add_executable(QuicRtpTranslator ${SOURCES})

# Libraries
target_link_libraries(QuicRtpTranslator
    srtp2
    msquic
    pthread
    redis++
)
