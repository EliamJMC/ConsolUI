#include "ConsolUIVisualElements.h"


namespace ConsolUI::VisualElements {

	//---------------//
	//-- Component --//
	//---------------//

	Position Component::absolute_Position() const {
		if (!Component::parent) return Component::relative_Position;
		Position p = Component::parent->absolute_Position();
		return { Component::relative_Position.x + p.x, Component::relative_Position.y + p.y };
	}
	void Component::setParent(Component* par) {
		Component::parent = par;
	}

	//-----------//
	//-- Label --//
	//-----------//

	void Label::assignMode(Mode mode, ModeValue assign) {

	}
	void Label::assignMode(initializer_list<pair<Mode, ModeValue>> list) {

	}
	void Label::assignMode(Orientation _orient = Orientation::Horizontal, Color _color = Color::wht, bool _centered = true, int _space = 1) {

	}

	//-----------//
	//-- Empty --//
	//-----------//

	void Empty::addChild(unique_ptr<Component>&& child) {

	}

	//----------//
	//-- Menu --//
	//----------//

	void Menu::addInteractable(unique_ptr<Label> label, Behaviour next = nullptr) {

	}
}
