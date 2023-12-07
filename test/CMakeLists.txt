cmake_minimum_required(VERSION 3.18)
project(xd_rt_test)

add_subdirectory(../3rdparty/googletest test.out)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_package(TBB REQUIRED)
include_directories(${TBB_INCLUDE_DIRS})
find_package(embree 4 REQUIRED)
include_directories(${EMBREE_INCLUDE_DIRS})

set(NON_BUILD_FILES
        ../.clang-format
        ../eigen.natvis
)
add_executable(xd_rt_test
        SamplerTest.cpp
        FilmTest.cpp
        CameraTest.cpp
        ModelHitTest.cpp
        VisibilityTest.cpp
        DistributionTest.cpp
        LightTest.cpp
        MaterialTest.cpp
        TextureTest.cpp
        HitRecordTest.cpp
        TriangleTest.cpp
        ObjLoaderTest.cpp
        AABBTest.cpp
        HitAccelTest.cpp
        GLTFLoaderTest.cpp
        PrimitiveTest.cpp
        #        TriangulateTest.cpp
        ${NON_BUILD_FILES}
        HitSolverTest.cpp
        FloatWithErrorTest.cpp
        MathUtilTest.cpp
)

target_link_libraries(xd_rt_test xd_rt)
target_link_libraries(xd_rt_test gtest gtest_main)
target_link_libraries(xd_rt_test TBB)
target_link_libraries(xd_rt_test ${EMBREE_LIBRARIES})

add_custom_command(TARGET xd_rt_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${TBB_ROOT}/bin"
        $<TARGET_FILE_DIR:xd_rt_test>
)
add_custom_command(TARGET xd_rt_test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${EMBREE_ROOT_DIR}/bin"
        $<TARGET_FILE_DIR:xd_rt_test>
)