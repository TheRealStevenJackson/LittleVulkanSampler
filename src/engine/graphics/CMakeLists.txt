# Mesh and Material moved to src/core/asset. Renderer is an interface
# so existing target_link_libraries(Renderer) still get Mesh/Material via Assets.
add_library(Renderer INTERFACE)
target_link_libraries(Renderer INTERFACE Assets)
