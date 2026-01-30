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

/*Auxiliary Classes*/
template<typename _Ty>
class Hierarchy;

/*Base component*/
class Component;

/*Conteiners*/
class Empty;
class Menu;

/*Specific component*/
class Label;
class Field;

/*Visual Sys Elements*/
class Window;
class System;

void set_Color(Color color, string text);
void set_Color(Color color, int number);

//-- auxialiar class hierarchy --//
template<typename _Ty>
class Hierarchy {
public:
    using Path = Hierarchy<_Ty>;
    using Integration = variant<Path, vector<Path>>;
    Hierarchy() = default;

    Hierarchy(unique_ptr<_Ty> act, vector<Path> nxts = {})
        : actual(move(act)), nexts(move(nxts))
    {
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
            nexts[i].previous = (i == 0) ? actual.get() : nexts[i-1].actual.get();
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

//---------------------------//
//-- BASE CLASS : COMPONENT--//
//---------------------------//
class Component {
public:
    Component(Position p, Size s)
        : relative_Position(p), size(s) {
        parent = nullptr;
    }

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

    virtual void debug() {
        Position abs_Pos = absolute_Position();
        cout << "Component at (" << abs_Pos.x << "," << abs_Pos.y << ") "
             << "Size (" << size.width << "x" << size.height << ")\n";
	}
};

//-----------------------//
//-- CLASS : CONTEINER --// ok
//-----------------------//

class Empty : public Component {
public:
    Empty(Position p, Size s)
        : Component(p, s) {
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
    }
    enum class Orientation { Vertical, Horizontal };
    enum class Mode { Centered, Spaced, Colored, Oriented };
    using ModeValue = variant<bool, int, Color, Orientation>;

    /* Renders the Label with the selected mode. */
    void render() override {
        if (!parent) return;
        Position parent_Position = parent->absolute_Position();
        Position position_Absolute = absolute_Position();
        string output = text;

        if (spaced() && !text.empty()) {
            output.clear();
            for (size_t i = 0; i < text.length(); i++) {
                output.push_back(text[i]);
                if (i != text.length() - 1)
                    output.append(spacement, ' ');
            }
        }

        setcolor(color);
        if (orientation == Orientation::Horizontal) {
            position_Absolute.x = centered ?
                parent_Position.x + (parent->size.width - output.length()) / 2 :
                position_Absolute.x;

            gotoxy(static_cast<size_t>(position_Absolute.x), static_cast<size_t>(position_Absolute.y));
            cout << output;
        }
        else {
            position_Absolute.y = centered ?
                parent_Position.y + (parent->size.height - output.length()) / 2 :
                position_Absolute.y;

            for (size_t i = 0; i < output.length(); i++) {
                gotoxy(static_cast<size_t>(position_Absolute.x), static_cast<size_t>(position_Absolute.y + (1 * i)));
                cout << output[i];
            }
        }
        setcolor(Color::wht);
    }

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

    void assignMode(Orientation _orient = Orientation::Horizontal,Color _color = Color::wht, bool _centered = true, int _space = 1 ) {
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
private:
    string text;
    Orientation orientation = Orientation::Horizontal;
    Color color = Color::wht;

    bool centered = false;
    bool spaced() const { return spacement > 0 ? true : false; }
    int spacement = 0;
};

//---------------------// 
//-- CLASS : TEXTBOX --// a lot bugs
//---------------------//

class Field : public Component {
public:
    Field(Position p = { 0,0 }, Size s = { 3,7 })
        : Component(p, s) {
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
        while (key != 13) {
            key = verify_character();
            if (key == 8) {
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
        if ((ch >= 'A' && ch <= 'Z') || ch >= 'a' && ch <= 'z' || ch == 8 || ch == 13 ) return ch;
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
    }
    using Behaviour = function<void()>;

    void render() override {
        for (auto &child : children) 
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

        updateSelectionColor(Color::gry);
        selected_Index = newIndex;
        updateSelectionColor(Color::wht);
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
private:
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

//--------------------//
//-- CLASS : WINDOW --// works but...
//--------------------//

class Window {
protected:
    string name;
public:
    Window(string _name)
        : root({ 0,0 }, { 30,120 }), name(_name), focus(nullptr) {
    }

    Empty root;

    bool visible = false;

    void handleInput(char key, System& system);

    void open() { 
        root.render(); 
        set_Focus_Stack();
    }

    virtual void debug() {
        cout << "Window: " << name << "\n" 
			<< "State: " << (visible ? "visible" : "hiden") << "\n";
        if (focus != nullptr)  focus->debug();
        else set_Color(Color::aqua, "no focus found\n");
        
        root.debug();
	}

private:
    void set_Focus_Stack() {
        for (auto &child : root.children) {
            if (child->handleInput(0) == true)
                focus_Stack.push_back(child.get());
        }
        if (!focus_Stack.empty())
            focus = focus_Stack[0];
    }
    Component* focus;
    vector<Component*> focus_Stack;
};

//---------------------//
//-- CLASS : CATALOG --// review
//---------------------//

class System {
public:
    Hierarchy<Window> winHierarchy;

    Window* current = nullptr;

    void run() {
        refreshSystem();
        current = winHierarchy.actual.get();
        while (winHierarchy.actual != nullptr) {
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
            current->visible = true;
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

// Implementation del metodo de window
void Window::handleInput(char key, System& system) {
    if (!visible) return;
    if (key == 27) {
        system.previousWin();
        return;
    }
    
    if (focus && focus->handleInput(key)) { return; }
}

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
    System _system;
    auto main_Window    = make_unique<Window>("Main Window");
    auto start_Window   = make_unique<Window>("Start Window");
    auto options_Window = make_unique<Window>("Options Window");

    // ===== MAIN PAGE =====
    auto title = make_unique<Label>("MAIN WINDOW", Position{ 0,0 });
    title->assignMode({ 
        {Label::Mode::Centered, true},
        {Label::Mode::Spaced, 2} 
        });

    auto menu = make_unique<Menu>(Position{ 0,4 });

    auto l_start    = make_unique<Label>("Start", Position{ 0,0 });
    auto l_options  = make_unique<Label>("Options", Position{ 0,3 });
    auto l_exit     = make_unique<Label>("Exit", Position{ 0,6 });

    menu->addItem(move(l_start),    [&]() { _system.nextWin(0); });
    menu->addItem(move(l_options),  [&]() { _system.nextWin(1); });
    menu->addItem(move(l_exit),     [&]() { exit(0); });

    main_Window->root.addChild(move(title));
    main_Window->root.addChild(move(menu));

    // ===== START PAGE =====
    auto start_label = make_unique<Label>("START WINDOW", Position{ 0,0 });
    start_label->assignMode(Label::Orientation::Vertical, Color::grn, true, 2);

    auto start_textBox = make_unique<Field>(Position{ 10,7 });
    start_Window->root.addChild(move(start_label));
    start_Window->root.addChild(move(start_textBox));

    // ===== OPTIONS PAGE =====
    auto opt_label = make_unique<Label>("OPTIONS WINDOW", Position{ 0,0 });
    opt_label->assignMode({
        {Label::Mode::Centered, true},
        {Label::Mode::Spaced, 2}
        });
    options_Window->root.addChild(move(opt_label));

	vector<Hierarchy<Window>> temp_nexts;
	temp_nexts.emplace_back(move(start_Window), vector<Hierarchy<Window>>{});
	temp_nexts.emplace_back(move(options_Window), vector<Hierarchy<Window>>{});

    _system.winHierarchy.setHierarchy(
        move(main_Window), 
		move(temp_nexts)
        );
    _system.run();
}