cmake_minimum_required (VERSION 3.21)

include(test_common)

set (__CurTargetName__ DownloadManager)

set(DLM_PROTOCOLBUFFER_LIB $ENV{DLM_PROTOCOLBUFFER_LIB})
set(DLM_ADSKPROXYMGR_LIB_RELEASE $ENV{DLM_ADSKPROXYMGR_LIB_RELEASE})
set(DLM_ADSKPROXYMGR_X64_LIB_RELEASE $ENV{DLM_ADSKPROXYMGR_X64_LIB_RELEASE})
set(DLM_ADSKPROXYMGR_X64_LIB_DEBUG $ENV{DLM_ADSKPROXYMGR_X64_LIB_DEBUG})
set(DLM_ADSKPROXYMGR_LIB_DEBUG $ENV{DLM_ADSKPROXYMGR_LIB_DEBUG})
set(DLM_LIBCURL_LIB $ENV{DLM_LIBCURL_LIB})
set(DLM_OPENSSL_LIB $ENV{DLM_OPENSSL_LIB})
set(DLM_LIBCURL_LIB_RELEASE $ENV{DLM_LIBCURL_LIB_RELEASE})
set(DLM_LIBCURL_LIB_DEBUG $ENV{DLM_LIBCURL_LIB_DEBUG})
set(DLM_OPENSSL_LIB $ENV{DLM_OPENSSL_LIB})
set(DLM_THIRD_PARTY_ROOT ${CMAKE_SOURCE_DIR}/third_party)
set(SECURE_SETTING_LIB_DIR ${CMAKE_SOURCE_DIR}/external/secure_settings/bin)


if(${CMAKE_CONFIGURATION_TYPES} STREQUAL “Release”)
    set(SECURE_SETTING_LIB_DIR_BUILD_CONFIG ${SECURE_SETTING_LIB_DIR}/Release)
  else()
    set(SECURE_SETTING_LIB_DIR_BUILD_CONFIG ${SECURE_SETTING_LIB_DIR}/Debug)
endif()

set(DLM_SOURCE_FILES
  main.cpp
  downloader.cpp
  task_scheduler.cpp
  command.cpp
  transaction_validator.h
  transaction_validator.cpp
  ipc_request_hanlder.cpp
  ipc_request_channel.cpp
  transaction_manager.cpp
  transaction.cpp
  download_task.cpp
  download_group_item_wrapper.cpp
  transaction_manager.cpp
  decompress_task.cpp
  task_cache.cpp
  command_cache.cpp
  downloader.h
  task_scheduler.h
  command.h
  transaction.h
  ipc_request_handler.h
  ipc_request_channel.h
  downloader_error_codes.h
  transaction_manager.h
  download_task.h
  task.h
  transaction_manager.h
  task_state_listener.h
  task_state_change_provider.h
  task_progress_listener.h
  task_progress_provider.h
  download_group_item_wrapper.h
  decompress_task.h
  task_cache.h
  command_cache.h
)

if (LINUX)
  set(DLM_SOURCE_FILES
    ${DLM_SOURCE_FILES}
    ${DLM_THIRD_PARTY_ROOT}/jsoncpp/jsoncpp.cpp
  )
  set(JSON_INCLUDE_DIR
    ${DLM_THIRD_PARTY_ROOT}/jsoncpp
    ${DLM_THIRD_PARTY_ROOT}/jsoncpp/json
  )
endif()

add_executable(${__CurTargetName__}
  ${DLM_SOURCE_FILES}
  ${DLM_SRC_DIR}/common/source/xplat/xplat_feature_manager.cpp
  ${DLM_SRC_DIR}/common/source/xplat/xplat_feature_manager_impl.cpp
)

link_directories($ENV{DLM_BOOST_LIB})

add_definitions(-DCURL_STATICLIB -DADSKINT_DECLARE_STATIC -D_ADSKDLMLIB_STATIC -DUNICODE -DENABLE_LOG_)

target_include_directories(${__CurTargetName__} PUBLIC
  $ENV{DLM_BOOST_INCLUDE}
  $ENV{DLM_ADSKPROXYMGR_INCLUDE}
  $ENV{DLM_LIBCURL_INCLUDE}
  $ENV{DLM_PROTOCOLBUFFER_INCLUDE}
  $ENV{DLM_IPC_INCLUDE}
  ${CMAKE_SOURCE_DIR}/src/downloader
  ${CMAKE_SOURCE_DIR}/src/downloader_library
  ${CMAKE_SOURCE_DIR}/src/resource_monitor
  ${CMAKE_SOURCE_DIR}/src/util
  ${CMAKE_SOURCE_DIR}/src/include
  ${JSON_INCLUDE_DIR}
)

ADD_BOOST()
find_package(Boost COMPONENTS thread chrono atomic filesystem program_options log locale REQUIRED)
find_library(WSOCK32_LIBRARY wsock32)
find_library(WS2_32_LIBRARY ws2_32)
find_library(CRYPT32_LIBRARY Crypt32)
find_library(WLDAP32_LIBRARY Wldap32)

#dependencies
add_dependencies(${__CurTargetName__} DownloadManagerStatic)
add_dependencies(${__CurTargetName__} DownloadManagerUtil)
add_dependencies(${__CurTargetName__} ResourceMonitor)

if(APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-lambda-capture")
endif()

if (WIN32)
  if(${RELEASE})
    TARGET_LINK_LIBRARIES(${__CurTargetName__}
    "${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/DownloadManagerStatic.lib"
    "${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/DownloadManagerUtil.lib"
    "${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/ResourceMonitor.lib"
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${DLM_PROTOCOLBUFFER_LIB}/release/libprotobuf.lib
    ${DLM_ADSKPROXYMGR_X64_LIB_RELEASE}/AdskProxyManager.lib
    ${DLM_LIBCURL_LIB}/release/libcurl_a.lib
    ${DLM_OPENSSL_LIB}/Release/libssl.lib
    ${DLM_OPENSSL_LIB}/Release/libcrypto.lib
    $ENV{DLM_IPC_LIB_x64_RELEASE}/ipc.lib
    wsock32 ws2_32 Crypt32 Wldap32 shlwapi
  )
  else()
	TARGET_LINK_LIBRARIES(${__CurTargetName__}
	"${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/DownloadManagerStatic.lib"
	"${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/DownloadManagerUtil.lib"
	"${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/ResourceMonitor.lib"
	${Boost_LIBRARIES}
	${CMAKE_DL_LIBS}
	${DLM_PROTOCOLBUFFER_LIB}/debug/libprotobufd.lib
	${DLM_ADSKPROXYMGR_X64_LIB_DEBUG}/AdskProxyManager.lib
	${DLM_LIBCURL_LIB}/debug/libcurl_a_debug.lib
	${DLM_OPENSSL_LIB}/Debug/libssl.lib
	${DLM_OPENSSL_LIB}/Debug/libcrypto.lib
	$ENV{DLM_IPC_LIB_x64_DEBUG}/ipc.lib
	wsock32 ws2_32 Crypt32 Wldap32 shlwapi
  )
  endif()
elseif(APPLE)
  if(${RELEASE})
    TARGET_LINK_LIBRARIES(${__CurTargetName__}
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerStatic.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerUtil.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libResourceMonitor.a
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${DLM_PROTOCOLBUFFER_LIB}/release/libprotobuf.a
    ${DLM_ADSKPROXYMGR_LIB_RELEASE}/libAdskProxyManager.a
    ${DLM_LIBCURL_LIB}/release/libcurl.a
    ${DLM_OPENSSL_LIB}/Release/libssl.a
    ${DLM_OPENSSL_LIB}/Release/libcrypto.a
    $ENV{DLM_IPC_LIB_RELEASE}/libipc.a
    "-framework CoreFoundation -framework SystemConfiguration -framework Security -framework CFNetwork -framework Cocoa -lz -framework ExceptionHandling"
  )
  else()
    TARGET_LINK_LIBRARIES(${__CurTargetName__}
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerStatic.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerUtil.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libResourceMonitor.a
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${DLM_PROTOCOLBUFFER_LIB}/debug/libprotobuf.a
    ${DLM_ADSKPROXYMGR_LIB_DEBUG}/libAdskProxyManager.a
    ${DLM_LIBCURL_LIB}/release/libcurl.a
    ${DLM_OPENSSL_LIB}/Debug/libssl.a
    ${DLM_OPENSSL_LIB}/Debug/libcrypto.a
    $ENV{DLM_IPC_LIB_DEBUG}/libipc.a
    "-framework CoreFoundation -framework SystemConfiguration -framework Security -framework CFNetwork -framework Cocoa -lz -framework ExceptionHandling"
  )
  endif()
elseif(LINUX)
  set(SuppressWarningFlag "-w")
  set_source_files_properties(${DLM_THIRD_PARTY_ROOT}/jsoncpp/jsoncpp.cpp
                              PROPERTIES
                              COMPILE_FLAGS
                              ${SuppressWarningFlag})

  if(${RELEASE})
    TARGET_LINK_LIBRARIES(${__CurTargetName__}
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerStatic.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerUtil.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libResourceMonitor.a
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${DLM_PROTOCOLBUFFER_LIB}/release/libprotobuf.a
    ${DLM_LIBCURL_LIB}/release/libcurl.a
    ${DLM_OPENSSL_LIB}/Release/libssl.a
    ${DLM_OPENSSL_LIB}/Release/libcrypto.a
    ${DLM_ADSKPROXYMGR_LIB_RELEASE}/libAdskProxyManagerStatic_fPIC.a
    $ENV{DLM_IPC_LIB_RELEASE}/libipc.a
  )
  else()
    TARGET_LINK_LIBRARIES(${__CurTargetName__}
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerStatic.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libDownloadManagerUtil.a
    ${DLM_OUTPUT_DIRECTORY_BASE}/lib/${CMAKE_CONFIGURATION_TYPES}/libResourceMonitor.a
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    ${DLM_PROTOCOLBUFFER_LIB}/debug/libprotobuf.a
    ${DLM_LIBCURL_LIB}/release/libcurl.a
    ${DLM_OPENSSL_LIB}/Debug/libssl.a
    ${DLM_OPENSSL_LIB}/Debug/libcrypto.a
    ${DLM_ADSKPROXYMGR_LIB_DEBUG}/libAdskProxyManagerStatic_fPIC.a
    $ENV{DLM_IPC_LIB_DEBUG}/libipc.a
  )
  endif()
endif()

#===============================================================================
# Set project version
#===============================================================================
DLM_SET_PROJECT_VERSION_INFO("${__CurTargetName__}")

#===============================================================================
# Add secure setting Library
#===============================================================================
ADD_SECURE_SETTINGS_LIBRARY(${__CurTargetName__})

DLM_ADD_CER_UTILS(${__CurTargetName__})

ADD_OPENSSL_LIBRARY(${__CurTargetName__})

if(WIN32)
   add_custom_command(TARGET ${__CurTargetName__} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -E
    copy
    ${SECURE_SETTING_LIB_DIR_BUILD_CONFIG}/DeviceSettingsProvider.dll
    ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()
