cmake_minimum_required (VERSION 3.14)

set(CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)


if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

#project ("Ivorys")
project ("Client")
project ("FileServer")

set(OPENSSL_INCLUDE_DIR "C:/Program Files/OpenSSL-Win64/include")
set(OPENSSL_LIBRARIES "C:/Program Files/OpenSSL-Win64/lib")

find_package(OpenSSL REQUIRED)


#add_executable (Ivorys "src/Ivorys.cpp" "headers/Ivorys.h"  "headers/Echo.h")
add_executable (Client "src/Client.cpp")
add_executable (FileServer "src/Ivorys.cpp" "headers/Ivorys.h" "headers/FileServer.h" "src/FileServer.cpp")

#target_include_directories("Ivorys" PRIVATE ${PROJECT_SOURCE_DIR}/headers)
#target_include_directories("Ivorys" PRIVATE C:/libs/asio-1.30.2/include)

target_include_directories("Client" PRIVATE ${PROJECT_SOURCE_DIR}/headers)
target_include_directories("Client" PRIVATE C:/libs/asio-1.30.2/include)
target_compile_features("Client" PRIVATE cxx_std_23)

target_include_directories("FileServer" PRIVATE ${PROJECT_SOURCE_DIR}/headers)
target_include_directories("FileServer" PRIVATE C:/libs/asio-1.30.2/include)
target_compile_features("FileServer" PRIVATE cxx_std_23)
target_link_libraries("FileServer" OpenSSL::SSL OpenSSL::Crypto)