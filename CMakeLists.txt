cmake_minimum_required(VERSION 3.12)
project(corosync-quorum-notify)

set(SOURCE_FILES corosync-quorum-notify.c)

add_executable(corosync-quorum-notify ${SOURCE_FILES})

target_link_libraries(corosync-quorum-notify quorum systemd cpg)
target_compile_options(corosync-quorum-notify PRIVATE -Wall)

# Install the binary to /usr/bin
install(TARGETS corosync-quorum-notify
        RUNTIME DESTINATION bin
)

# Install the service and default files using CMake
install(FILES corosync-quorum-notify.default
        DESTINATION /etc/default
)

# Optionally, you can also install the service file to systemd directory
install(FILES corosync-quorum-notify.service
        DESTINATION /lib/systemd/system
)

# Set the install prefix to /usr (required for Debian packaging)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "Install prefix" FORCE)
endif()

function(get_git_version output_var)
    execute_process(
        COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE git_version
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    string(SUBSTRING "${git_version}" 1 -1 git_version)
    set(${output_var} "${git_version}" PARENT_SCOPE)
endfunction()

# Call the function to get the Git version
set(GIT_VERSION "unknown")
get_git_version(GIT_VERSION)
message(STATUS "Application is at version ${GIT_VERSION}")

# Generate the necessary debian files
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "corosync-quorum-notify")
set(CPACK_PACKAGE_VERSION "${GIT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Jérôme Poulin <jeromepoulin@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "corosync")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "Launch an application when corosync changes state")
include(CPack)
