cmake_minimum_required(VERSION 3.12)
project(corosync-quorum-notify)

set(SOURCE_FILES corosync-quorum-notify.c)

add_executable(corosync-quorum-notify ${SOURCE_FILES})

target_link_libraries(corosync-quorum-notify quorum systemd cpg)
target_compile_options(corosync-quorum-notify PRIVATE -Wall)
