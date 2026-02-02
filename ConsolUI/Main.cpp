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
#include <queue>

#include "db.h"
#include "cvm 25.h"

using namespace std;

/*Position X , Y*/
struct Position { int x, y; };
/*Size Height , Width*/
struct Size { int height, width; };

void set_Color(Color color, string text);
void set_Color(Color color, int number);

namespace ConsolUI {
    namespace VisualElements {

        enum class Type {Component, Empty, Label, Field, Menu};
        class Component;
        class Empty;
		class Label;
		class Field;
		class Menu;

        //---------------------------//
        //-- BASE CLASS : COMPONENT--// ok
        //---------------------------//
        class Component {
        public:
            Component(Position p, Size s)
                : relative_Position(p), size(s) {
                parent = nullptr;
				type = Type::Component;
            }
			Type type;
            Component* parent = nullptr;
            Position relative_Position;
            Size size;

            virtual ~Component() = default;

            Position absolute_Position() const {
                if (!parent) return relative_Position;
                Position p = parent->absolute_Position();
                return { relative_Position.x + p.x, relative_Position.y + p.y };
            }

            void setParent(Component* par) { parent = par; }

            virtual bool handleInput(char key) { return false; }

            virtual void render() = 0;

            virtual void debug() {}
        };

        //-----------------------//
        //-- CLASS : CONTEINER --// ok
        //-----------------------//

        class Empty : public Component {
        public:
            Empty(Position p, Size s)
                : Component(p, s) {
                type = Type::Empty;
            }

            vector<unique_ptr<Component>> children;

            vector<unique_ptr<Component>>& getChildren() { return children; }

            /*
            * Sets parent pointer and adds child unique pointer.
            * @param child Any object that heritates form Component
            */
            void addChild(unique_ptr<Component>&& child) {
                child->setParent(this);
                children.push_back(move(child));
            }

            /* Renders every child component on screen.*/
            void render() override {
                for (const auto& child : children)
                    child->render();
            }

            void debug() override {
                Position abs_Pos = absolute_Position();
                cout << "Empty at (" << abs_Pos.x << "," << abs_Pos.y << ") "
                    << "Size (" << size.width << "x" << size.height << ")\n"
                    << "Children:\n";
                for (const auto& child : children)
                    child->debug();
            }
        };

        //-------------------//
        //-- CLASS : LABEL --// ok
        //-------------------//

        class Label : public Component {
        public:
            Label(string txt, Position p = { 0,0 }, Size s = { 0,0 })
                : Component(p, s), text(txt) {
                type = Type::Label;
            }
            enum class Orientation { Vertical, Horizontal };
            enum class Mode { Centered, Spaced, Colored, Oriented };
            using ModeValue = variant<bool, int, Color, Orientation>;

            void assignMode(Mode mode, ModeValue assign) {
                switch (mode) {
                case Mode::Centered:
                    centered = get<bool>(assign);
                    break;
                case Mode::Spaced:
                    spacement = get<int>(assign);
                    break;
                case Mode::Colored:
                    color = get<Color>(assign);
                    break;
                case Mode::Oriented:
                    orientation = get<Orientation>(assign);
                    break;
                }
            }

            void assignMode(initializer_list<pair<Mode, ModeValue>> list) {
                for (auto& [mode, value] : list) {
                    switch (mode) {
                    case Mode::Centered:
                        centered = get<bool>(value);
                        break;
                    case Mode::Spaced:
                        spacement = get<int>(value);
                        break;
                    case Mode::Colored:
                        color = get<Color>(value);
                        break;
                    case Mode::Oriented:
                        orientation = get<Orientation>(value);
                        break;
                    }
                }
            }

            void assignMode(Orientation _orient = Orientation::Horizontal, Color _color = Color::wht, bool _centered = true, int _space = 1) {
                orientation = _orient;
                color = _color;
                centered = _centered;
                spacement = _space;
            }

            void debug() override {
                Position abs_Pos = absolute_Position();
                set_Color(Color::pur, "Label "); cout << "at ("; setcolor(Color::grn); cout << abs_Pos.x << "," << abs_Pos.y;    setcolor(Color::wht); cout << ") "
                    << "Size (";      setcolor(Color::grn); cout << size.width << "x" << size.height; setcolor(Color::wht); cout << ") "
                    << "Text: \"";    setcolor(Color::grn); cout << text;                             setcolor(Color::wht); cout << "\"\n"
                    << "Modes:\n"
                    << "Orientation: "; setcolor(Color::grn); cout << (orientation == Orientation::Horizontal ? "Horizontal" : "Vertical") << "\n"
                    << "Color: ";
                setcolor(color);
                switch (color) {
                case Color::blk:  cout << "Black";  break;
                case Color::blu:  cout << "Blue";   break;
                case Color::grn:  cout << "Green";  break;
                case Color::aqua: cout << "Aqua";   break;
                case Color::red:  cout << "Red";    break;
                case Color::pur:  cout << "Purple"; break;
                case Color::yel:  cout << "Yellow"; break;
                case Color::wht:  cout << "White";  break;
                case Color::gry:  cout << "Gray";   break;
                default: cout << "Unknown"; break;
                }
                setcolor(Color::wht);
                cout << "\n";
                if (centered) cout << "Centered: ";  set_Color(Color::blu, "True\n");
                if (spaced()) cout << "Spaced: ";    set_Color(Color::blu, "True\n");
                cout << "Spacement: "; set_Color(Color::blu, spacement); cout << "\n";

            }
            string text;
            Orientation orientation = Orientation::Horizontal;
            Color color = Color::wht;

            bool centered = false;
            bool spaced() const { return spacement > 0 ? true : false; }
            int spacement = 0;
        private:
        };

        //---------------------// 
        //-- CLASS : TEXTBOX --// a lot bugs
        //---------------------//

        class Field : public Component {
        public:
            Field(Position p = { 0,0 }, Size s = { 3,7 })
                : Component(p, s) {
                type = Type::Field;
            }

            void render() override {
                for (int y = 0; y < size.height; ++y) {
                    for (int x = 0; x < size.width; ++x) {
                        gotoxy(static_cast<size_t>(absolute_Position().x + x), static_cast<size_t>(absolute_Position().y + y));
                        if (y == 0 || y == size.height - 1)
                            cout << '-';
                        else if (x == 0 || x == size.width - 1)
                            cout << '|';
                    }
                }
            }

            bool handleInput(char key) override {
                handle_Text_Input(static_cast<optional<char>>(key));
                return true;
            }

            void handle_Text_Input(optional<char> key) {
                gotoxy(static_cast<size_t>(absolute_Position().x + 1), static_cast<size_t>(absolute_Position().y + size.height / 2));
                bool handling = false;
                while (handling) {
                    key = verify_character();
					if (key == 27 || key == 13) {
                        handling = false;
                    }
                    else if (key == 8) {
                        if (!word.empty()) {
                            word.pop_back();
                            cout << "\b \b";
                        }
                    }
                    else if (key.has_value() && word.length() < static_cast<size_t>(size.width) - 2) {
                        word.push_back(key.value());
                        cout << key.value();
                    }
                }


            }

            virtual void debug() override {
                Position abs_Pos = absolute_Position();
                cout << "Field at (" << abs_Pos.x << "," << abs_Pos.y << ") "
                    << "Size (" << size.width << "x" << size.height << ") "
                    << "Current Text: \"" << word << "\"\n";
            }

        private:
            optional<char> verify_character() {
                char ch = db.noacc(_getch());
                if ((ch >= 'A' && ch <= 'Z') || ch >= 'a' && ch <= 'z' || ch == 8 || ch == 13 || ch == 27) return ch;
                else return nullopt;
            }

            string word;
        };

        //------------------//
        //-- CLASS : MENU --// ok
        //------------------//

        class Menu : public Component {
        public:
            Menu(Position p = { 0,0 }, Size s = { 30,120 })
                : Component(p, s) {
				type = Type::Menu;
            }
            using Behaviour = function<void()>;

            void render() override {
                for (auto& child : children)
                    child.label->render();
            }

            void switchSelection(char key) {
                if (children.empty()) return;

                if (key == 13) {
                    if (children[selected_Index].behaviour) children[selected_Index].behaviour();
                    return;
                }

                int dir = (key == 'w') ? -1 : (key == 's') ? 1 : 0;
                if (dir == 0) return;

                size_t newIndex = clamp(
                    static_cast<int>(selected_Index) + dir,
                    0,
                    static_cast<int>(children.size() - 1)
                );
                if (newIndex == selected_Index) return;

                updateSelectionColor(Color::wht);
                selected_Index = newIndex;
                updateSelectionColor(Color::gry);
            }

            bool handleInput(char key) override {
                switchSelection(key);
                return true;
            }

            void addItem(unique_ptr<Label> label, Behaviour next = nullptr) {
                if (label == nullptr) return;
                label->setParent(this);
                children.emplace_back(Interactable{ move(label), next });

                if (children.size() == 1)
                    children[0].label.get()->assignMode(Label::Mode::Colored, Color::gry);
            }

            void debug() override {
                auto current_child = (children[selected_Index].label.get());

                Position abs_Pos = absolute_Position();
                cout << "Menu at (" << abs_Pos.x << "," << abs_Pos.y << ") "
                    << "Size (" << size.width << "x" << size.height << ") "
                    << "Selected Index: " << selected_Index << "\n"
                    << "Courrent selection:\n";
                current_child->debug();
                cout << "Menu Labels:\n";
                int index = 0;
                for (const auto& x_child : children)
                    index++,
                    cout << "Label " << index << ":\n",
                    x_child.label->debug();
            }
            void updateSelectionColor(Color color) {
                children[selected_Index].label->assignMode(Label::Mode::Colored, color);
                children[selected_Index].label->render();
            }

            struct Interactable {
                unique_ptr<Label> label;
                Behaviour behaviour;
            };

            vector<Interactable> children;
            size_t selected_Index = 0;

        };

    };
    namespace Systems {
        template<typename _Ty>
        class Hierarchy;
        class System;
        class Window;

        //-- auxialiar class hierarchy --//
        template<typename _Ty>
        class Hierarchy {
        public:
            using Path = Hierarchy<_Ty>;
            using Integration = variant<Path, vector<Path>>;

            Hierarchy() = default;
            Hierarchy(unique_ptr<_Ty> act, vector<Path> nxts = {})
                : actual(move(act)), nexts(move(nxts)) {
                for (auto& n : nexts)
                    n.previous = actual.get();
            }

            unique_ptr<_Ty> actual;
            vector<Path>nexts = {};
            _Ty* previous = nullptr;

            void setHierarchy(unique_ptr<_Ty> main, vector<Path> _nexts) {
                actual = move(main);
                nexts = move(_nexts);
                for (size_t i = 0; i < nexts.size(); ++i) {
                    nexts[i].previous = actual.get();
                    if (nexts[i].actual)
                        nexts[i].setHierarchy(move(nexts[i].actual), move(nexts[i].nexts));
                }
            }

            void add(const _Ty& target, Integration&& add) {
                if (!actual) return;
                if (actual.get() == &target) {
                    insert(move(add), nexts);
                    return;
                }
                for (auto& next : nexts) {
                    next.add(target, move(add));
                }
            }

            Path* findPath(const _Ty& target) {
                if (!actual) return nullptr;
                if (actual.get() == &target) {
                    return this;
                }
                for (auto& next : nexts) {
                    Path* res = next.findPath(target);
                    if (res != nullptr)
                        return res;
                }
                return nullptr;
            }

            _Ty* findActual(const _Ty& target) {
                if (!actual) return nullptr;
                if (actual.get() == &target) {
                    return actual.get();
                }
                for (auto& next : nexts) {
                    _Ty* res = next.findActual(target);
                    if (res != nullptr)
                        return res;
                }
                return nullptr;
            }

            Hierarchy(Hierarchy&&) = default;
            Hierarchy& operator=(Hierarchy&&) = default;

        private:
            void insert(Integration&& add, vector<Path>& target) {
                if (holds_alternative<Path>(add)) {
                    for (auto& _path : target)
                        if (_path.actual.get() == get<Path>(add).actual.get())
                            return;
                    target.push_back(move(get<Path>(add)));
                }
                else {
                    auto _vec = get<vector<Path>>(add);
                    for (auto& _path : _vec) {
                        bool exists = false;
                        for (auto& existing : target) {
                            if (existing.actual.get() == _path.actual.get()) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) target.push_back(std::move(_path));
                    }
                }
            }

        };

        //--------------------//
        //-- CLASS : WINDOW --// ok
        //--------------------//

        class Window {
        public:
            string name;
            using Interactable = variant <
                VisualElements::Menu*,
                VisualElements::Field*,
                monostate
            >;
            Window(string _name)
                : root({ 0,0 }, { 30,120 }), name(_name), focus(monostate{}) {
            }


            VisualElements::Empty root;
            bool visible = false;

            void handleInput(char key, System& system);

            void open() {
                visible = true;
                root.render();
                findInteractableElements();
            }

            virtual void debug() {
                cout << "Window: " << name << "\n"
                    << "State: " << (visible ? "visible" : "hiden") << "\n";

				
                if (holds_alternative<VisualElements::Menu*>(focus)) { 
                    auto* m = std::get<VisualElements::Menu*>(focus); 
                    cout << "Focus: Menu -> " << "\n"; 
                }
                else if (holds_alternative<VisualElements::Field*>(focus)) { 
                    auto* f = std::get<VisualElements::Field*>(focus); 
                    cout << "Focus: Field -> " << "\n"; 
                }
                else { set_Color(Color::aqua, "no focus found\n"); }

                root.debug();
            }

            Interactable focus;
            vector<Interactable> interactableElements;

        private:
            void findInteractableElements() {
                interactableElements.clear(); for (auto& child : root.children) {
                    if (child->handleInput(0)) { 
                        if (auto* m = dynamic_cast<VisualElements::Menu*>(child.get())) 
                            interactableElements.push_back(m); 
                        else if (auto* f = dynamic_cast<VisualElements::Field*>(child.get())) 
                            interactableElements.push_back(f); 
                    } 
                } 
                if (!interactableElements.empty()) 
                    focus = interactableElements[0]; 
                else 
                    focus = monostate{}; 
            }
        };

        //--------------------//
        //-- CLASS : System --// ok
        //--------------------//

        class System {
        public:
            Hierarchy<Window> winHierarchy;

            Window* current = nullptr;
			bool running = false;

            void run() {
                current = winHierarchy.actual.get();
                refreshSystem();
				running = true;
                while (running) {
                    if (winHierarchy.actual == nullptr)
						running = false;
                    char key = _getch();
                    current->handleInput(key, *this);
                }
            }

            void nextWin(size_t index) {
                Hierarchy<Window>::Path* path = winHierarchy.findPath(*current);
                if (path == nullptr) return;
                if (index >= path->nexts.size()) return;
                current->visible = false;
                current = path->nexts[index].actual.get();
                current->visible = true;
                refreshSystem();
            }

            void previousWin() {
                Hierarchy<Window>::Path* path = winHierarchy.findPath(*current);
                if (path == nullptr) return;
                current->visible = false;
                if (path->previous != nullptr) {
                    current->visible = false;
                    current = path->previous;
                    refreshSystem();
                }
                else exit(0);
            }

        private:
            void refreshSystem() {
                clrscr();
                if (current) current->open();
            }
        };

		// handleInput implementation
        void Systems::Window::handleInput(char key, System& system) {
            if (!visible) return;
            if (key == 27) {
                system.previousWin();
                return;
            }

            visit([&](auto ptr) {
                using T = decay_t<decltype(ptr)>;
                if constexpr (!is_same_v<T, monostate>) {
                    if (ptr && ptr->handleInput(key))
                        return;
                }
            },focus);
        }
	};

	//--------------//
	//-- MANAGERS --// Warning : Disaster area
	//--------------//

    namespace Managers {
		struct Event;
        class EventManager;
        class RenderManager;
        class InputManager;
        class ComponentManager;
        class NavigationManager;

        struct Event {
            using Reference =
                variant<
                VisualElements::Label*,
                VisualElements::Field*,
                VisualElements::Menu*,
                VisualElements::Empty
                >;
            enum class Type {
                RunProgram,
                SystemExit,
                KeyPress,

				MenuNavigationRequest,
                WindowsNavigationRequest,
                FocusChangingRequest,
                RenderRequest,

                ComponentModificationRequest,

                Error,
                Null
            };

            Type type = Type::Null;

            struct keyPress { char key; };
            struct error { int error_Code = 0; string message; };

            struct windowsNavigationRequest { enum class Direction { Previous, next } diretion; size_t index = 0; };
            struct menuNavigationRequest { char key; };
            struct focusChangingRequest { string from_Component; string to_Component; };
            struct renderRequest { 
                enum class Type { start, refresh, update, null }; 
                Type type = Type::null;

                struct refreshData { string mss; };
                struct updateData { Reference component; };
                variant<refreshData, updateData> renderData;
            };

            struct componentModificationRequest { 
                VisualElements::Type type;  
                struct labelData { 
                    variant<
                        string,
                        VisualElements::Label::Orientation,
                        Color,
                        bool,
                        int
                    > data;
                };
                struct fieldData { string mss; };
                struct emptyData { string mss; };
                struct menuData  { int index;  };
                variant<
                    labelData,
                    fieldData,
                    emptyData,
                    menuData
                > data; 
                Reference component;
            };

			variant<
                keyPress, 
                error,
                
                menuNavigationRequest,
                windowsNavigationRequest, 
                renderRequest, 
                focusChangingRequest,

                componentModificationRequest
            > data;
		};

        //---------------------------//
		//-- CLASS : EVENT MANAGER --// ok
		//---------------------------//

        class EventManager {
        public:
            void push(Event e) {
                events.push_back(e);
            }

            void pushWarning(Event e) {
                events.push_front(e);
            }

            bool poll(Event& out) {
                if (events.empty()) return false;
                out = events.front();
                events.pop_front();
                return true;
            }
        private:
            deque<Event> events;
        };

        //---------------------------//
		//-- CLASS : INPUT MANAGER --// ok
		//---------------------------// 

        class InputManager { 
        public:
            void poll(EventManager& events) {
                if (!_kbhit()) return;
                char key = _getch();
				events.push(Event{ Event::Type::KeyPress, Event::keyPress{ key } });
            }
        };

        class NavigationManager {
        public:
			Systems::Hierarchy<Systems::Window>* hierarchy;
			Systems::Window* current;

            void changeWindow(EventManager& events) {
                Event event;
                if (!events.poll(event)) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 001, "Failed polling event"}
                        }
                    );
                return;
            }
                if (event.type != Event::Type::WindowsNavigationRequest) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 101, "Event is not a Windows Navigation Request"}
                        }
                    );
                    return;
                } 

                auto data = get<Event::windowsNavigationRequest>(event.data);
                Systems::Hierarchy<Systems::Window>::Path* path = hierarchy->findPath(*current);
                if (path == nullptr) {
					events.pushWarning(Event{ 
                        Event::Type::Error, 
                        Event::error{ 102, "Failed finding current path in hierarchy"} 
                        }
                    );
                    return;
                }

                current->visible = false;
                if (data.diretion == Event::windowsNavigationRequest::Direction::next) {
                    if (data.index >= path->nexts.size()) {
						events.pushWarning(Event{ 
                            Event::Type::Error, 
                            Event::error{ 103, "Next window index out of bounds"} 
                            }
                        );
                        return;
                    }
                    current = path->nexts[data.index].actual.get();
                }
                else {
                    if (path->previous != nullptr)
                        current = path->previous;
                    else
                        events.pushWarning(Event{ Event::Type::SystemExit });
                }
                current->visible = true;
				events.push(Event{ 
                    Event::Type::RenderRequest, 
                    Event::renderRequest{ 
                        Event::renderRequest::Type::refresh, 
                        Event::renderRequest::refreshData { "Refresh -> " + current->name }}
                    }
                );
            }

            void navigateMenu(EventManager& events) {
                enum Direction { Up, Down, Null } direction;
                bool stopNav = false;

                Event event;
                if (!events.poll(event)) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 001, "Failed polling event" }
                        }
                    );
                    return;
                }
                if (event.type != Event::Type::MenuNavigationRequest) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 111, "Event is not a Menu Navigation Request" }
                        }
                    );
                    return;
                }
                auto data = get<Event::menuNavigationRequest>(event.data);

                if (!holds_alternative<VisualElements::Menu*>(current->focus)) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 112, "Current focus is not a Menu" }
                        }
                    );
                    return;
                }
                auto* menu = get<VisualElements::Menu*>(current->focus);

                direction =
                    (data.key == 'w') ? Up :
                    (data.key == 's') ? Down :
                    Null;
                if (direction == Null) return;

                size_t newIndex = clamp(
                    static_cast<int>(menu->selected_Index) + (direction == Up ? -1 : 1),
                    0,
                    static_cast<int>(menu->children.size() - 1)
                );

                if (newIndex == menu->selected_Index) return;
                events.push(Event{
                    Event::Type::ComponentModificationRequest,
                    Event::componentModificationRequest{ 
                        VisualElements::Type::Label, 
                        Event::componentModificationRequest::labelData{ Color::gry },
                        menu->children[menu->selected_Index].label.get() }
                    }
                );

                events.push(Event{
                    Event::Type::RenderRequest,
                    Event::renderRequest{
                        Event::renderRequest::Type::update,
                        Event::renderRequest::updateData{ 
                            menu->children[menu->selected_Index].label.get() } }
                    }
                );

                menu->selected_Index = newIndex;

                events.push(Event{
                    Event::Type::ComponentModificationRequest,
                    Event::componentModificationRequest{
                        VisualElements::Type::Label,
                        Event::componentModificationRequest::labelData{ Color::wht },
                        menu->children[menu->selected_Index].label.get() }
                    }
                );

                events.push(Event{
                    Event::Type::RenderRequest,
                    Event::renderRequest{ 
                        Event::renderRequest::Type::update,
                        Event::renderRequest::updateData{ 
                            menu->children[menu->selected_Index].label.get() } }
                    }
                );
            }
        };

        class RenderManager {
        public:
			Systems::Window* current;
			bool clearScreen = true;
            void startRender(EventManager& events) {
				Event event;
				if (!events.poll(event))
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 001, "Failed polling event"}
                        }
					);
                if (event.type != Event::Type::RenderRequest) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 201, "Event is not a Render Request"}
                        }
					);
                }
                if (clearScreen) {
                    clrscr();
					clearScreen = false;
                }

                renderEmpty(current->root, events);
				clearScreen = true;
			}

        private:
            void renderEmpty(VisualElements::Empty& _empty, EventManager& events) {
                for (auto& child : _empty.children) {
                    switch (child->type) {
                        case VisualElements::Type::Empty: {
                            auto empty = dynamic_cast<VisualElements::Empty*>(child.get());
                            renderEmpty(*empty, events);
                            break;
                        }
                        case VisualElements::Type::Label: {
                            auto label = dynamic_cast<VisualElements::Label*>(child.get());
                            renderLabel(label, events);
                            break;
                        }
                        case VisualElements::Type::Field: {
                            auto field = dynamic_cast<VisualElements::Field*>(child.get());
                            renderField(field, events);
                            break;
                        }
                        case VisualElements::Type::Menu: {
                            auto menu = dynamic_cast<VisualElements::Menu*>(child.get());
                            renderMenu(menu, events);
                            break;
                        }
                    }
                }
            }

            void renderLabel(VisualElements::Label* label, EventManager& events) {
                if (!label->parent) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 202, "Label has no parent assigned"}
                        }
                    );
                    return;
                }
                Position parent_Position = label->parent->absolute_Position();
                Position position_Absolute = label->absolute_Position();
                string output = label->text;

                if (label->spaced() && !label->text.empty()) {
                    output.clear();
                    for (size_t i = 0; i < label->text.length(); i++) {
                        output.push_back(label->text[i]);
                        if (i != label->text.length() - 1)
                            output.append(label->spacement, ' ');
                    }
                }

                setcolor(label->color);
                if (label->orientation == VisualElements::Label::Orientation::Horizontal) {
                    position_Absolute.x = label->centered ?
                        parent_Position.x + (label->parent->size.width - output.length()) / 2 :
                        position_Absolute.x;

                    gotoxy(static_cast<size_t>(position_Absolute.x), static_cast<size_t>(position_Absolute.y));
                    cout << output;
                }
                else {
                    position_Absolute.y = label->centered ?
                        parent_Position.y + (label->parent->size.height - output.length()) / 2 :
                        position_Absolute.y;

                    for (size_t i = 0; i < output.length(); i++) {
                        gotoxy(static_cast<size_t>(position_Absolute.x), static_cast<size_t>(position_Absolute.y + (1 * i)));
                        cout << output[i];
                    }
                }
                setcolor(Color::wht);
            }

            void renderField(VisualElements::Field* field, EventManager& events) {
                for (int y = 0; y < field->size.height; ++y) {
                    for (int x = 0; x < field->size.width; ++x) {
                        gotoxy(static_cast<size_t>(field->absolute_Position().x + x), static_cast<size_t>(field->absolute_Position().y + y));
                        if (y == 0 || y == field->size.height - 1)
                            cout << '-';
                        else if (x == 0 || x == field->size.width - 1)
                            cout << '|';
                    }
                }
            }

            void renderMenu(VisualElements::Menu* menu, EventManager& events) {
                for (auto& child : menu->children) {
                    auto label = child.label.get();
                    renderLabel(label, events);
                }
            }
        };

        class ComponentManager {

        };
	};

    class Controller {
        public:
        unique_ptr<Managers::EventManager> event_Manager;
        unique_ptr<Managers::InputManager> input_Manager;
        unique_ptr<Managers::NavigationManager> navigation_Manager;
		unique_ptr<Managers::RenderManager> render_Manager;
		unique_ptr<Managers::ComponentManager> component_Manager;

		unique_ptr<Systems::Hierarchy<Systems::Window>> hierarchy;
		unique_ptr<Systems::Window> current;
    };

    namespace vse = VisualElements;
    namespace sys = Systems;
};
namespace cui = ConsolUI;

void set_Color(Color color, string text) {
    setcolor(color);
    cout << text;
	setcolor(Color::wht);
}

void set_Color(Color color, int number) {
    setcolor(color);
    cout << number;
    setcolor(Color::wht);
}

//------------------------------//
//-- MAIN : PROGRAM EXECUTION --//
//------------------------------//
 
int main() {
    cui::sys::System _system;
    auto main_Window    = make_unique<cui::sys::Window>("Main Window");
    auto start_Window   = make_unique<cui::sys::Window>("Start Window");
    auto options_Window = make_unique<cui::sys::Window>("Options Window");

    // ===== MAIN PAGE =====
    auto title = make_unique<cui::vse::Label>("MAIN WINDOW", Position{ 0,0 });
    title->assignMode({ 
        {cui::vse::Label::Mode::Centered, true},
        {cui::vse::Label::Mode::Spaced, 2}
        });

    auto menu = make_unique<cui::vse::Menu>(Position{ 0,4 });

    auto l_start    = make_unique<cui::vse::Label>("Start", Position{ 0,0 });
    auto l_options  = make_unique<cui::vse::Label>("Options", Position{ 0,3 });
    auto l_exit     = make_unique<cui::vse::Label>("Exit", Position{ 0,6 });

    menu->addItem(move(l_start),    [&]() { _system.nextWin(0); });
    menu->addItem(move(l_options),  [&]() { _system.nextWin(1); });
    menu->addItem(move(l_exit),     [&]() { exit(0); });

    main_Window->root.addChild(move(title));
    main_Window->root.addChild(move(menu));

    // ===== START PAGE =====
    auto start_label = make_unique<cui::vse::Label>("START WINDOW", Position{ 0,0 });
    start_label->assignMode(cui::vse::Label::Orientation::Vertical, Color::grn, true, 2);

    auto start_textBox = make_unique<cui::vse::Field>(Position{ 10,7 });
    start_Window->root.addChild(move(start_label));
    start_Window->root.addChild(move(start_textBox));

    // ===== OPTIONS PAGE =====
    auto opt_label = make_unique<cui::vse::Label>("OPTIONS WINDOW", Position{ 0,0 });
    opt_label->assignMode({
        {cui::vse::Label::Mode::Centered, true},
        {cui::vse::Label::Mode::Spaced, 2}
        });
    options_Window->root.addChild(move(opt_label));

	vector<cui::sys::Hierarchy<cui::sys::Window>> temp_nexts;
	temp_nexts.emplace_back(move(start_Window), vector<cui::sys::Hierarchy<cui::sys::Window>>{});
	temp_nexts.emplace_back(move(options_Window), vector<cui::sys::Hierarchy<cui::sys::Window>>{});

    _system.winHierarchy.setHierarchy(
        move(main_Window), 
		move(temp_nexts)
        );
    _system.run();
}