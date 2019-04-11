project(loggenerator VERSION 1.0)

add_executable(loggenerator
        $<TARGET_OBJECTS:sims>
        $<TARGET_OBJECTS:common>
        $<TARGET_OBJECTS:geo>
        src/main.cpp)

target_link_libraries(loggenerator geo common sims)
