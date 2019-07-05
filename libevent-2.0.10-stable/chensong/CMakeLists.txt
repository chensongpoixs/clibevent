project(Server)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
MESSAGE(STATUS "#########################")
SET(CMAKE_BUILD_TYPE Release)
#ADD_DEFINITIONS(-std=c++11 -g -Wall )
#
#SET(EXTRA_INC_DIR /home/ext)
#
INCLUDE_DIRECTORIES(
					${PROJECT_SOURCE_DIR}/../include
					)


				
				
#SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux/lib )
#SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/linux)

LINK_DIRECTORIES(
				${PROJECT_SOURCE_DIR}/../lib
				)
file(GLOB ALL_CXX_SRCS1
		main.c
		)
		
file(GLOB ALL_CXX_SRCS2
		cthread_event.c
		)


set(ALL_LIB
		pthread
		libevent.a  
		libevent_core.a  
		libevent_extra.a  
		libevent_openssl.a  
		libevent_pthreads.a
		)

add_executable(main ${ALL_CXX_SRCS1})
target_link_libraries(main ${ALL_LIB})
add_executable(cthread_event ${ALL_CXX_SRCS2})
target_link_libraries(cthread_event ${ALL_LIB})

				

