#include <XYZ.h>
#include <XYZ/Core/EntryPoint.h>

#include "ServerLayer.h"


class ServerApp : public XYZ::Application
{
public:
	ServerApp()
	{
		PushLayer(new XYZ::ServerLayer());
	}

};

XYZ::Application* CreateApplication()
{
	return new ServerApp();
}
