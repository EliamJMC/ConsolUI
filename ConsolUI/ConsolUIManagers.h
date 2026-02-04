#pragma once

#include "ConsolUI.h"

namespace ConsolUI::Managers {
	class Request_Manager;
	class Call_Manager;
	class Event_Manager;
	class Input_Manager;

	namespace Navigation {
		class Window_Manager;
		class Menu_Manager;
	}

	class Render_Manager;

	class Component_Manager;
};
namespace mgr = ConsolUI::Managers;
namespace nav = ConsolUI::Managers::Navigation;