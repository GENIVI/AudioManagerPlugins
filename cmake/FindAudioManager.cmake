find_path(AUDIOMANAGER_CMAKE_CONFIG_PATH 
                 audiomanagerConfig.cmake
                 PATH_SUFFIXES audiomanager/cmake
                 PATHS
                 ${CMAKE_INSTALL_PATH}
                 "/usr/lib"
                 "/usr/local/lib"
                 )
                 
find_path(AUDIOMANAGER_INCLUDE_FOLDER 
                 audiomanagertypes.h
                 PATH_SUFFIXES audiomanager
                 PATHS
                 ${CMAKE_INSTALL_PATH}
                 "/usr/include"
                 "/usr/local/include"
                 )

set( AUDIOMANAGER_FOUND "NO" )

if(AUDIOMANAGER_CMAKE_CONFIG_PATH)
    set( AUDIOMANAGER_FOUND "YES" )
    message(STATUS "Found AudioManager cmake config: ${AUDIOMANAGER_CMAKE_CONFIG_PATH}/audiomanagerConfig.cmake")
endif(AUDIOMANAGER_CMAKE_CONFIG_PATH)

mark_as_advanced(
  AUDIOMANAGER_CMAKE_CONFIG_PATH
  AUDIOMANAGER_INCLUDE_FOLDER
)
