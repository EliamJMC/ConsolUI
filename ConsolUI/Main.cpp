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
        //-- BASE CLASS : COMPONENT--// 
        //---------------------------//

        struct Component {
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
        };

        //-----------------------//
        //-- CLASS : CONTEINER --// ok
        //-----------------------//

        struct Empty : public Component {
            Empty(Position p, Size s)
                : Component(p, s) {
                type = Type::Empty;
            }

            vector<unique_ptr<Component>> children;

            void addChild(unique_ptr<Component>&& child) {
                child->setParent(this);
                children.push_back(move(child));
            }
        };

        //-------------------//
        //-- CLASS : LABEL --// preaty clean
        //-------------------//

        struct Label : public Component {
            Label(string txt, Position p = { 0,0 }, Size s = { 0,0 })
                : Component(p, s), text(txt) {
                type = Type::Label;
            }
            enum class Orientation { Vertical, Horizontal };
            enum class Mode { Centered, Spaced, Colored, Oriented };
            using ModeValue = variant<bool, int, Color, Orientation>;

            string text;
            bool centered = false;
            int spacement = 0;
            bool spaced() const { return spacement > 0 ? true : false; }
            Orientation orientation = Orientation::Horizontal;
            Color color = Color::wht;

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
        };

        //---------------------// 
        //-- CLASS : TEXTBOX --// lot of work to add
        //---------------------//

        struct Field : public Component {
            Field(Position p = { 0,0 }, Size s = { 3,7 })
                : Component(p, s) {
                type = Type::Field;
            }
            string word;
        };

        //------------------//
        //-- CLASS : MENU --//
        //------------------//

        struct Menu : public Component {
            Menu(Position p = { 0,0 }, Size s = { 30,120 })
                : Component(p, s) {
				type = Type::Menu;
            }
            using Behaviour = function<void()>;
            struct Interactable { unique_ptr<Label> label; Behaviour behaviour; };

            vector<Interactable> children;
            size_t selected_Index = 0;

            void addInteractable(unique_ptr<Label> label, Behaviour next = nullptr) {
                if (label == nullptr) return;
                label->setParent(this);
                children.emplace_back(Interactable{ move(label), next });

                if (children.size() == 1)
                    children[0].label.get()->assignMode(Label::Mode::Colored, Color::gry);
            }
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
                VisualElements::Field*
            >;
            Window(string _name)
                : root({ 0,0 }, { 30,120 }), name(_name), focus() {
            }


            VisualElements::Empty root;
            bool visible = false;

            void handleInput(char key, System& system);

            Interactable focus;
            vector<Interactable> interactablElements;
        };

        //--------------------//
        //-- CLASS : System --// ok
        //--------------------//

        class System {
        public:
            Hierarchy<Window> winHierarchy;

            Window* current = nullptr;
			bool running = false;
        };
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
                enum class Type { render, update, null }; 
                Type type = Type::null;

                struct refreshData { string mss; };
                struct updateData { Reference component; };
                variant<refreshData, updateData> data;
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

            Event getEvent(size_t index) {
                return events[index];
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
                        Event::renderRequest::Type::render, 
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
            void render(EventManager& events) {
				Event event;
                if (!events.poll(event)) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 001, "Failed polling event"}
                        }
                    );
                    return;
                }
                if (event.type != Event::Type::RenderRequest) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 200, "Event is not a Render Request"}
                        }
					);
                    return;
                }
                if (holds_alternative<Event::renderRequest::Type>(event.data)) {
                    if (Event::renderRequest::Type::render == get<Event::renderRequest>(event.data).type) {
                        events.pushWarning(Event{
                            Event::Type::Error,
                            Event::error{ 201, "Event is not an Rendering Request"}
                            }
                        );
                        return;
                    }
                }

                if (clearScreen) {
                    clrscr();
					clearScreen = false;
                }

                renderEmpty(current->root, events);
                clearScreen = true;
			}

            void update(EventManager& events) {
                Event event;
                if (!events.poll(event)) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 001, "Failed polling event"}
                        }
                    );
                    return;
                }
                if (event.type != Event::Type::RenderRequest) {
                    events.pushWarning(Event{
                        Event::Type::Error,
                        Event::error{ 200, "Event is not a Render Request"}
                        }
                    );
                    return;
                }
                if (holds_alternative<Event::renderRequest>(event.data)) {
                    auto data = get<Event::renderRequest>(event.data);
                    if (Event::renderRequest::Type::update == data.type) {
                        events.pushWarning(Event{
                            Event::Type::Error,
                            Event::error{ 211, "Event is not an Update Request"}
                            }
                        );
                        return;
                    }

                    if (auto* rr = get_if<Event::renderRequest>(&event.data)) {
                        if (rr->type == Event::renderRequest::Type::update) {
                            if (auto* upd = get_if<Event::renderRequest::updateData>(&rr->data)) {
                                Event::Reference ref = upd->component;
                                std::visit([&](auto ptr) {
                                    using T = std::decay_t<decltype(ptr)>;

                                    if constexpr (is_same_v<T, VisualElements::Label*>) {
                                        renderLabel(ptr, events);
                                    }
                                    else if constexpr (is_same_v<T, VisualElements::Field*>) {
                                        renderField(ptr, events);
                                    }
                                    else if constexpr (is_same_v<T, VisualElements::Menu*>) {
                                        renderMenu(ptr, events);
                                    }
                                    else if constexpr (is_same_v<T, VisualElements::Empty>) {
                                        renderEmpty(ptr, events);
                                    }

                                }, ref);
                            }
                        }
                    }
                }
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
            Systems::Window* current;
            vector<VisualElements::Component*> currentInteractables;
            
            void searchInteractables() {
                for (auto& child : current->root.children) {
                    if (child->type == VisualElements::Type::Menu || child->type == VisualElements::Type::Field)
                        currentInteractables.push_back(child.get());
                }
            }
        };
	};

    class Controller {
    public:
        struct BobbleEvent {
            Managers::Event next;
            Managers::Event current;
            Managers::Event previous;

            void push(Managers::Event newEvent) {
                previous = current;
                current = next;
                next = newEvent;
            }
        };

        enum state {
            Rendering,
            Typing,
        };

        unique_ptr<Managers::EventManager> event_Manager;
        unique_ptr<Managers::InputManager> input_Manager;
        unique_ptr<Managers::NavigationManager> navigation_Manager;
		unique_ptr<Managers::RenderManager> render_Manager;
		unique_ptr<Managers::ComponentManager> component_Manager;

		unique_ptr<Systems::Hierarchy<Systems::Window>> hierarchy;
		unique_ptr<Systems::Window> current;
        
        BobbleEvent currentBobble = {};
        bool running;

        void run() {
            event_Manager->push(Managers::Event{
                Managers::Event::Type::RunProgram
                }
            );
            currentBobble.push(event_Manager->getEvent(0));

            event_Manager->push(Managers::Event{
                Managers::Event::Type::RenderRequest,
                Managers::Event::renderRequest{
                    Managers::Event::renderRequest::Type::render,
                    Managers::Event::renderRequest::refreshData{ "First Render" }
                    }
                }
            );
            currentBobble.push(event_Manager->getEvent(1));

            input_Manager->poll(*event_Manager); 

            currentBobble.push(event_Manager->getEvent(2));

            running = true;

            while (running) {

            }
        }

        void assignEvent(BobbleEvent bobble) {
            if (bobble.current.type == Managers::Event::Type::ComponentModificationRequest && 
                bobble.next.type == Managers::Event::Type::RenderRequest) {

            }
        }

        void verifyProgamState() {

        }

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

    menu->addInteractable(move(l_start),    [&]() {});
    menu->addInteractable(move(l_options), [&]() {});
    menu->addInteractable(move(l_exit),     [&]() { exit(0); });

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
}