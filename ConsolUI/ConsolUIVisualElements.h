#pragma once

#include <iostream>
#include <conio.h>
#include <cstdlib>
#include <string>
#include <functional>
#include <vector>   
#include <stack>
#include <memory>
#include <optional>
#include <algorithm>
#include <utility>
#include <initializer_list>
#include <variant>

#include "db.h"
#include "cvm 25.h"

#include "ConsolUI.h"

// A Visual Element Just Contains Data That Can Be Used For Different Things
namespace ConsolUI::VisualElements {
	enum class Type{
		Component,

		Label,
		Field,

		Empty,
		Menu
	};
	struct Component {
		virtual ~Component() = default;

		Type type;
		Component* parent;

		Size size;
		Position relative_Position;
		Position absolute_Position() const;

		void setParent(Component* par);
	};

	//Simple data
	struct Label : Component {
		enum class Orientation{ Vertical, Horizontal };
		enum class Mode { Centered, Spaced, Colored, Oriented };
		using ModeValue = variant<bool, int, Color, Orientation>;

		string text;
		bool centered = false;
		int spacement = 0;
		bool spaced() const { return spacement > 0 ? true : false; }
		Orientation orientation = Orientation::Horizontal;
		Color color = Color::wht;

		void assignMode(Mode mode, ModeValue assign);
		void assignMode(initializer_list<pair<Mode, ModeValue>> list);
		void assignMode(Orientation _orient = Orientation::Horizontal, Color _color = Color::wht, bool _centered = true, int _space = 1);
	};
	struct Field : Component {

		string word;

	};

	//Conteiners
	struct Empty : Component {

		std::vector<unique_ptr<Component>> children;

		void addChild(unique_ptr<Component>&& child);

	};
	struct Menu : Component {

		using Behaviour = function<void()>;
		struct Interactable { unique_ptr<Label> label; Behaviour behaviour; };

		vector<Interactable> children;
		size_t selected_Index = 0;

		void addInteractable(unique_ptr<Label> label, Behaviour next = nullptr);

	};
};
namespace vse = ConsolUI::VisualElements;