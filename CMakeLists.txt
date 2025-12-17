cmake_minimum_required(VERSION 3.20)
project(toddler_meal_generator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(tmg
  src/main.cpp
  src/meal_db.cpp
)

target_include_directories(tmg PRIVATE src)
target_link_libraries(tmg PRIVATE nlohmann_json::nlohmann_json)
