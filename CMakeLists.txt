cmake_minimum_required(VERSION 3.10)

# set the project name
project(RT)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# find all .cpp files in the src directory
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

# add the executable
add_executable(RT ${SOURCE_FILES})

# find and include SDL2, SDL_image, and GLM
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

# threading

find_package(TBB REQUIRED)
target_link_libraries(${PROJECT_NAME} TBB::tbb)

# link libraries
target_link_libraries(RT ${SDL2_LIBRARIES} SDL2_image ${GLM_LIBRARIES})
