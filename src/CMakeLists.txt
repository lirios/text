file(GLOB_RECURSE SOURCES *.cpp *.h qml.qrc)

add_executable(liri-text ${SOURCES})
target_link_libraries(liri-text
                      Qt5::Quick
                      KF5::TextEditor
                      KF5::CoreAddons)

install(TARGETS liri-text
        DESTINATION ${CMAKE_INSTALL_BINDIR})
