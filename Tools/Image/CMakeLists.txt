add_executable(ImageViewer ImageViewerMain.cpp)
target_link_libraries(ImageViewer AnKi)
installExecutable(ImageViewer)

add_executable(ImageImporter ImageImporterMain.cpp)
target_link_libraries(ImageImporter AnKi)
installExecutable(ImageImporter)
