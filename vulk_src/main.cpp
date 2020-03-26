//
// Created by alexa on 5/18/2019.
//

#define STB_IMAGE_IMPLEMENTATION
// NOTE: This define must exist in one and only one file
#define VMA_IMPLEMENTATION
//#include "Vertex.h"
#include "Application.h"
//#include "ExternalHeaders.h"
//#include "VulkanMemoryManager.h"
#include <iostream>
#include <stdexcept>

int main(int, char **) {
  using namespace std;
  Application app;
  try {
    app.run();
  } catch (exception &exception1) {
    cerr << exception1.what() << endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
