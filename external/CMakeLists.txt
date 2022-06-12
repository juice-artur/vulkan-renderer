add_library(vma INTERFACE)
add_library(glm INTERFACE)

add_library(vkbootstrap STATIC)
target_sources(vkbootstrap PRIVATE 
  vkbootstrap/VkBootstrap.h
  vkbootstrap/VkBootstrap.cpp
  )
target_include_directories(vkbootstrap PUBLIC vkbootstrap)

target_include_directories(vma INTERFACE vma)
target_include_directories(glm INTERFACE glm)