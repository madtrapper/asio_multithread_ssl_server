cmake_minimum_required(VERSION 3.0)
project(TestAsio)

# SET(CPP_STANDARD 17)
# SET(CPP_STANDARD_REQUIRED True)


find_package(OpenSSL REQUIRED)

add_executable(TestAsio TestAsio.cpp)
add_definitions(-DASIO_STANDALONE)
set_property(TARGET TestAsio PROPERTY CXX_STANDARD 17)

target_compile_definitions(TestAsio PRIVATE ASIO_STANDALONE)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_FUNCTION)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_TYPE_TRAITS)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_SHARED_PTR)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_ADDRESSOF)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_CSTDINT)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_CHRONO)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_ARRAY)
target_compile_definitions(TestAsio PRIVATE __APPLE__)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_STD_THREAD)
target_compile_definitions(TestAsio PRIVATE ASIO_HAS_THREADS)
#target_compile_definitions(TestAsio PRIVATE ASIO_HAS_CLANG_LIBCXX)

include_directories(${OPENSSL_INCLUDE_DIR})
target_link_libraries(TestAsio OpenSSL::SSL)
target_include_directories(TestAsio PUBLIC ${ASIO_INCLUDE})
