cmake_minimum_required(VERSION 3.24)
function(add_tutorial_app APP_NAME IDE_FOLDER)
    set_target_properties(${APP_NAME} PROPERTIES
            FOLDER ${IDE_FOLDER}
    )
    add_custom_command(TARGET ${APP_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${TBB_ROOT}/bin"
            $<TARGET_FILE_DIR:${APP_NAME}>
    )
    add_custom_command(TARGET ${APP_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${EMBREE_ROOT_DIR}/bin"
            $<TARGET_FILE_DIR:${APP_NAME}>
    )
    add_custom_command(TARGET ${APP_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${GLFW_LIB_DIR}/glfw3.dll"
            $<TARGET_FILE_DIR:${APP_NAME}>
    )
endfunction()
add_subdirectory(appBase)
add_subdirectory(basicGraphic)
add_subdirectory(basicCompute)