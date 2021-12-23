set(RES_DIR "resources")

# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

file(GLOB_RECURSE RES_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/${RES_DIR}/*
)

FILE(COPY ${RES_FILES} DESTINATION "${CMAKE_BINARY_DIR}/bin/resources")

add_library(test_util STATIC test_util.c)
target_include_directories(test_util PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_link_libraries(test_util PUBLIC cmocka knuutils)

add_executable(test all_tests.c test_knustring.c test_knuutils.c test_knuapi.c test_knulms.c)
target_link_libraries(test PRIVATE curl cmocka test_util knulms knuapi knuutils)
