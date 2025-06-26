# (C) Copyright 2018 by Autodesk, Inc.
#
# The information contained herein is confidential, proprietary to Autodesk,
# Inc., and considered a trade secret as defined in section 499C of the
# penal code of the State of California.  Use of this information by anyone
# other than authorized employees of Autodesk, Inc. is granted only under a
# written non-disclosure agreement, expressly prescribing the scope and
# manner of such use.
#
#######################################################################################
#
# Name:            CmakeList.txt
#
# Description:     cmake file list for manifest manager
#
#######################################################################################


cmake_minimum_required (VERSION 3.21)

include(common)

set(__CurTargetName__ ResourceMonitor)

link_directories($ENV{DLM_BOOST_LIB})

set(CURL_STATICLIB ON) 

set(SOURCE_FILES 
  cpu_information.h
  ${PLATFORM_INCLUDE_NAME}/cpu_information_impl.h
  ${PLATFORM_INCLUDE_NAME}/disk_information_impl.h
  ${PLATFORM_INCLUDE_NAME}/network_information_impl.h
  resource_monitor.h
  network_information.h
  resource_observers.h
  disk_information.h
  resource_monitor.cpp
  network_information.cpp
  disk_information.cpp
  traffic_control.cpp
  traffic_control.h
  ${PLATFORM_INCLUDE_NAME}/cpu_information_impl.cpp
  ${PLATFORM_INCLUDE_NAME}/disk_information_impl.cpp
  ${PLATFORM_INCLUDE_NAME}/network_information_impl.cpp
)

if(APPLE)
set(PLATFORM_SOURCE_FILES
 ${PLATFORM_INCLUDE_NAME}/mac_network_information.h
 ${PLATFORM_INCLUDE_NAME}/mac_network_information.mm
)
endif()

add_library(${__CurTargetName__} STATIC
  ${SOURCE_FILES}
  ${PLATFORM_SOURCE_FILES}
)

file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}/src/common/resource" COMMON_RESOURCE_DIR)

target_include_directories(${__CurTargetName__} PUBLIC
  ${DLM_BOOST_INCLUDE}
  ${CMAKE_SOURCE_DIR}/src/util
)
