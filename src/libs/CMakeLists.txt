add_subdirectory(common)
add_subdirectory(geo)

if(INSTALL_HTTP)
    add_subdirectory(http)
endif()

#add_subdirectory(sims)
#add_subdirectory(sims2)
#add_subdirectory(linklayer)
add_subdirectory(mpilib)
