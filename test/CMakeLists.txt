cmake_minimum_required(VERSION 3.18)
project(xd_rt_test)

add_subdirectory(../3rdparty/googletest test.out)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

find_package(TBB REQUIRED)

add_executable(xd_rt_test SamplerTest.cpp FilmTest.cpp CameraTest.cpp ModelHitTest.cpp VisibilityTest.cpp DistributionTest.cpp LightTest.cpp MaterialTest.cpp TextureTest.cpp HitRecordTest.cpp TriangleTest.cpp
        ObjLoaderTest.cpp
        AABBTest.cpp)
target_link_libraries(xd_rt_test xd_rt)
target_link_libraries(xd_rt_test gtest gtest_main)
target_link_libraries(xd_rt_test TBB::tbb)