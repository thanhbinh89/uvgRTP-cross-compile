project(uvgrtp_test)

enable_testing()
include(GoogleTest)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PRIVATE
            main.cpp
            test_1_version.cpp
            test_2_rtp.cpp
            test_3_rtcp.cpp
            test_4_formats.cpp
            test_5_srtp_zrtp.cpp
        )

if(MSVC)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
                GTest::GTestMain
                uvgrtp
                cryptlib
            )
else()
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
                GTest::GTestMain
                uvgrtp
                cryptopp
            )
endif()

gtest_add_tests(
        TARGET ${PROJECT_NAME}
)