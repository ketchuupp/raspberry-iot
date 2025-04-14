set(componentName Interfaces)

add_library(${componentName} INTERFACE)
target_include_directories(${componentName} INTERFACE include)