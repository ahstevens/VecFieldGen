// Our classes
#include "Engine.h"

int main(int argc, char * argv[]) 
{
	// Instantiate an engine to drive the application
	Engine *engine = new Engine(argc, argv);

	// Initialize the engine
	if(!engine->init())
	{
		fprintf(stderr, "Failed to Create OpenGL Context");
		return EXIT_FAILURE;
	}

	// Run the engine
	engine->mainLoop();

    return EXIT_SUCCESS;
}