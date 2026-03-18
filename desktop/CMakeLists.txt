cmake_minimum_required(VERSION 3.16)
project(Synote LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network Concurrent)
find_package(QtKeychain QUIET)

qt_standard_project_setup()

qt_add_resources(NotesAI_RESOURCES resources.qrc)

add_executable(Synote
    src/main.cpp
    src/AppWindow.h
    src/AppWindow.cpp
    src/db/Database.h
    src/db/Database.cpp
    src/sync/SyncClient.h
    src/sync/SyncClient.cpp
    src/settings/Settings.h
    src/settings/Settings.cpp
    src/settings/SettingsDialog.h
    src/settings/SettingsDialog.cpp
    src/settings/LoginDialog.h
    src/settings/LoginDialog.cpp
    src/settings/Keychain.h
    src/settings/Keychain.cpp
    src/ai/AIClient.h
    src/ai/AIClient.cpp
    src/ui/FlowLayout.h
    src/ui/FlowLayout.cpp
    src/ui/TagChip.h
    src/ui/TagChip.cpp
    ${NotesAI_RESOURCES}
)

target_link_libraries(Synote PRIVATE Qt6::Widgets Qt6::Sql Qt6::Network Qt6::Concurrent)
target_include_directories(Synote PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
if (QtKeychain_FOUND)
    target_link_libraries(Synote PRIVATE QtKeychain::QtKeychain)
    target_compile_definitions(Synote PRIVATE HAVE_QTKEYCHAIN=1)
endif()
