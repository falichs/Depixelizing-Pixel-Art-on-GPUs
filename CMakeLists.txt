cmake_minimum_required (VERSION 2.6)
project (DepixelizeOnGPU)

#set default options
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate installation target" FORCE)
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE)

# TODO: Windows vs Unix
if(WIN32)
  #add externals
  add_subdirectory ("${PROJECT_SOURCE_DIR}/external")

  # Windows
  include_directories("${PROJECT_SOURCE_DIR}/external/FreeImage/Source")
  include_directories("${PROJECT_SOURCE_DIR}/external/glfw-3.1.1/include")
  include_directories("${PROJECT_SOURCE_DIR}/external/glew-1.13.0/include")

  set (LIBS ${LIBS} glfw)
  set (LIBS ${LIBS} glew_s)
  set (LIBS ${LIBS} opengl32)
  set (LIBS ${LIBS} freeImageLib)
else()
  # Unix
  set (LIBS ${LIBS} glfw)
  set (LIBS ${LIBS} GLEW)
  set (LIBS ${LIBS} GL)
  set (LIBS ${LIBS} freeimage)
endif()

#set source files
set(DEPIXELIZE_SOURCES
src/main.cpp
src/CellGraphBuilder.h 	
src/CellGraphBuilder.cpp
src/Drawable.h
src/GaussRasterizer.h 	
src/GaussRasterizer.cpp
src/Image.h 			
src/Image.cpp
src/PixelArtRenderer.h 	
src/PixelArtRenderer.cpp
src/Shader.h			
src/Shader.cpp
src/SimilarityGraphBuilderFS.h 
src/SimilarityGraphBuilderFS.cpp
src/SimilarityGraphDebugToy	
src/SimilarityGraphDebugToy.cpp
src/VoronoiCellGraph3x3.h	
src/VoronoiCellGraph3x3.cpp
)

#add boost::program_options
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package( Boost 1.36.0 COMPONENTS program_options REQUIRED )
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
	# add the executable
	add_executable(DepixelizeOnGPU ${DEPIXELIZE_SOURCES})
	target_link_libraries (DepixelizeOnGPU ${LIBS} ${Boost_LIBRARIES})
	#copy shader folder after successful build
	add_custom_command(TARGET DepixelizeOnGPU POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:DepixelizeOnGPU>/../shaders)
endif()
