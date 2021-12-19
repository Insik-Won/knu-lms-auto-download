# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

add_library(knudef STATIC knudef.c)
target_include_directories(knudef PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_compile_options(knudef PRIVATE -Wall -Werror)

add_library(knuutils STATIC knuutils.c knustring.c)
target_include_directories(knuutils PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_compile_options(knuutils PRIVATE -Wall -Werror)

add_library(knuapi STATIC knuapi.c)
target_include_directories(knuapi PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_compile_options(knuapi PRIVATE -Wall -Werror)

add_library(knulms STATIC knulms.c)
target_include_directories(knulms PUBLIC ${CMAKE_SOURCE_DIR}/includes)
target_compile_options(knulms PRIVATE -Wall -Werror)

find_package(LibXml2 REQUIRED)
target_link_libraries(knuapi PUBLIC knudef knuutils curl LibXml2::LibXml2)

target_link_libraries(knulms PUBLIC knudef)
target_link_libraries(knulms PRIVATE knuapi knuutils curl)

include (FetchContent)

FetchContent_Declare(
  Json
  GIT_REPOSITORY "https://github.com/json-c/json-c"
  GIT_TAG "json-c-0.15-20200726"
)
FetchContent_MakeAvailable(Json)

target_link_libraries(knuapi PRIVATE json-c)
target_link_libraries(knulms PRIVATE json-c)
