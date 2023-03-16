add_executable(radon-tf
  radon-tf.cpp
)


target_link_libraries(radon-tf -pthread ${OpenCV_LIBS})