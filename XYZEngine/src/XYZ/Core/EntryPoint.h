#pragma once
#include "Logger.h"
#include "Application.h"




extern XYZ::Application* CreateApplication();

/** Application entry point */
int main(int argc, char** argv)
{
	auto app = CreateApplication();
	app->Run();
	delete app;
	XYZ::Audio::ShutDown();
}
