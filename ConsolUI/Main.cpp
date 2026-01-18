#include <iostream>
#include <conio.h>
#include <cstdlib>
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <stack>
#include <memory>
#include <optional>
#include <algorithm>
#include <utility>

#include "db.h"
#include "cvm 25.h"

using namespace std;
using Action = function<void()>;

/*Position X , Y*/
struct Position { int x, y; };
/*Size Height , Width*/
struct Size { int height, width; };

class Component;
class Conteiner;
class Label;
class Menu;
class TextBox;
class Window;
class Catalog;

//---------------------------//
//-- BASE CLASS : COMPONENT--//
//---------------------------//

class Component {
protected:
    Component* parent = nullptr;
public:
    Position relative_Position;
    Size size;

    Component(Position p, Size s)
        : relative_Position(p), size(s) {
        parent = nullptr;
    }

    virtual ~Component() = default;

    Position absolute_Position() const {
        if (!parent) return relative_Position;
        Position p = parent->absolute_Position();
        return { relative_Position.x + p.x,
                 relative_Position.y + p.y };
    }

    virtual void render() = 0;

    virtual bool handle_Input(char key) { return false; }

    void setParent(Component* par) { parent = par; }
};

//-----------------------//
//-- CLASS : CONTEINER --//
//-----------------------//

class Conteiner : public Component {
public:
    Conteiner(Position p, Size s)
        : Component(p, s) {
    }

    /*
    * Sets parent pointer and adds child unique pointer.
    * @param child Any object that heritates form Component
    */
    void addChild(unique_ptr<Component>&& child) {
        child->setParent(this);
        children.push_back(move(child));
    }

    /* Finds all Components of the same class.*/
    template<typename Type>
    vector<Type*> findByType() {
        vector<Type*> filtered;
        for (const auto& child : children) {
            if (auto pointer = dynamic_cast<Type*>(child.get()))
                filtered.push_back(pointer);
        }
        return filtered;
    }

    /* Finds the first Component of a specific class */
    template<typename Type>
    Type* findFirstByType() {
        if (children.empty()) return nullptr;
        for (const auto& child : children) {
            if (auto pointer = dynamic_cast<Type*>(child.get()))
                return pointer;
        }
        return nullptr;
    }

    /* Renders every child component on screen.*/
    virtual void render() override {
        for (const auto& child : children)
            child->render();
    }

    vector<unique_ptr<Component>>& getChildren() {
        return children;
    }
private:
    vector<unique_ptr<Component>> children;
};

//-------------------//
//-- CLASS : LABEL --//
//-------------------//

class Label : public Component {
public:
    Label(string txt, Position p = { 0,0 }, Size s = { 0,0 })
        : Component(p, s), text(txt) {
    }
    enum Orientation { Vertical, Horizontal };

    /*
    * Set the color that will be printed on screen.
    * @param color Color that will be set.
    */
    void setColor(Color _color) { color = _color; }

    /* Renders the Label with the selected mode. */
    virtual void render() override {
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

    /*
    * Set Label's normal mode.
    * @param _centered The text will be placed in the midle of the parent component.
    * @param _spaced There will be a _spacement number of spaces between the letters.
    * @param _spacement The amount of spaces between the letters.
    */
    void setMode(bool _centered = false, int _spacement = 0) {
        centered = _centered; spacement = _spacement;
    }

    /*
    * Set Label's Special mode.
    * @param _color The color that will be printed on screen.
    * @param _orientation The orientation of the printing.
    */
    void setSpecialMode(Color _color, Orientation _orientation) {
        color = _color; orientation = _orientation;
    }

    string getText() { return text; }
private:
    string text;
    Orientation orientation = Orientation::Horizontal;
    Color color = Color::wht;

    bool centered = false;
    int spacement = 0;
    bool spaced() const {
        if (spacement > 0) return true;
        return false;
    }
};

//---------------------//
//-- CLASS : TEXTBOX --//
//---------------------//

class TextBox : public Component {
public:
    TextBox(Position p = { 0,0 }, Size s = { 3,7 }, Action act = nullptr)
        : Component(p, s), action(act) {
    }

    virtual void render() override {
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

    virtual bool handle_Input(char key) override {
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
                    cout << " \b";
                }
            }
            else if (key.has_value() && word.length() <= static_cast<size_t>(size.width) - 2) {
                word.push_back(key.value());
                cout << key.value();
            }
        }
    }
private:
    optional<char> verify_character() {
        char ch = db.noacc(_getch());
        if ((ch >= 'A' && ch <= 'Z') || ch >= 'a' && ch <= 'z' || ch == 8) return ch;
        else return nullopt;
    }

    Action action;
    string word;
};

//------------------//
//-- CLASS : MENU --//
//------------------//

class Menu : public Component {
public:

    Menu(Position p = { 0,0 }, Size s = { 30,120 })
        : Component(p, s) {
    }

    /* Renders every child (Label) component on screen.*/
    virtual void render() override {
        for (auto& item : items)
            item.label->render();
    }

    /*
    * Switch between the Labels in the menu.
    * @param key The letter that set the direction.
    */
    void switch_selection(char key) {
        if (items.empty()) return;

        if (key == 13) {
            if (items[selectedIndex].next)
                items[selectedIndex].next();
            return;
        }

        int dir =
            (key == 'w') ? -1 :
            (key == 's') ? 1 :
            0;

        if (dir == 0) return;

        size_t newIndex = std::clamp(
            static_cast<int>(selectedIndex) + dir,
            0,
            static_cast<int>(items.size() - 1)
        );

        if (newIndex == selectedIndex) return;

        items[selectedIndex].label->setColor(Color::wht);
        items[selectedIndex].label->render();

        selectedIndex = newIndex;

        items[selectedIndex].label->setColor(Color::gry);
        items[selectedIndex].label->render();
    }

    /* Adds a Label to the selector with two actions. */
    void addItem(Label* label, Action next = nullptr, Action back = nullptr) {
        items.emplace_back(label, next, back ? optional<Action>(back) : nullopt);
        label->setParent(this);

        if (items.size() == 1)
            items[0].label->setColor(Color::gry);
    }

    virtual bool handle_Input(char key) override {
        switch_selection(key);
        return true;
    }

private:
    struct Item {
        Label* label;
        Action next;
        optional<Action> back;
    };

    vector<Item> items;
    size_t selectedIndex = 0;
};

//--------------------//
//-- CLASS : WINDOW --//
//--------------------//

class Window {
protected:
    string name;
public:
    Window(string _name)
        : root({ 0,0 }, { 30,120 }), name(_name) {
    }

    Conteiner root;

    bool open = false;

    void handle_Input(char key, Catalog& catalog);

    void open_Window() { root.render(); }



private:
    Component* first_Focus() const {
        if (auto conteiner = const_cast<Conteiner&>(root).findFirstByType<Conteiner>()) {
            for (auto& child : conteiner->getChildren())
                if (auto menu = dynamic_cast<Menu*>(child.get()))
                    return menu;
                else if (auto textBox = dynamic_cast<TextBox*>(child.get()))
                    return textBox;
        }
        else if (auto menu = const_cast<Conteiner&>(root).findFirstByType<Menu>())
            return menu;
        else if (auto textBox = const_cast<Conteiner&>(root).findFirstByType<TextBox>())
            return textBox;
        else
            return nullptr;
    }
    Component* focus = first_Focus();
    stack<Component*> focus_Stack;

};

//---------------------//
//-- CLASS : CATALOG --//
//---------------------//

class Catalog {
public:
    void run() {
        update();
        while (!windowStack.empty()) {
            char key = _getch();
            current()->handle_Input(key, *this);
        }
    }

    void push(unique_ptr<Window> window) {
        if (!windowStack.empty())
            windowStack.top()->open = false;

        window->open = true;
        windowStack.push(move(window));
        update();
    }

    void pop() {
        if (windowStack.size() <= 1) {
            exit(0);
        }

        windowStack.top()->open = false;
        windowStack.pop();
        windowStack.top()->open = true;
        update();
    }

    Window* current() {
        return windowStack.empty() ? nullptr : windowStack.top().get();
    }

private:
    void update() {
        clrscr();
        if (current())
            current()->open_Window();
    }

    stack<unique_ptr<Window>> windowStack;
};

// Implementation del metodo de window
void Window::handle_Input(char key, Catalog& catalog) {
    if (!open) return;
    if (key == 27) {
        catalog.pop();
        return;
    }

    if (focus) focus->handle_Input(key);
}


//------------------------------//
//-- MAIN : PROGRAM EXECUTION --//
//------------------------------//

int main() {
    Catalog catalog;

    auto main_Window = make_unique<Window>("Main Window");
    auto start_Window = make_unique<Window>("Start Window");
    auto options_Window = make_unique<Window>("Options Window");

    // ===== MAIN PAGE =====
    auto title = make_unique<Label>("MAIN WINDOW", Position{ 0,0 });
    title->setMode(true, 2);

    auto menu = make_unique<Menu>(Position{ 0,4 });

    auto l_start = make_unique<Label>("Start", Position{ 0,0 });
    auto l_options = make_unique<Label>("Options", Position{ 0,3 });
    auto l_exit = make_unique<Label>("Exit", Position{ 0,6 });

    menu->addItem(l_start.get(), [&]() { catalog.push(move(start_Window)); });
    menu->addItem(l_options.get(), [&]() { catalog.push(move(options_Window)); });
    menu->addItem(l_exit.get(), [&]() { exit(0); });
    Menu* menu_ptr = menu.get();

    main_Window->root.addChild(move(title));
    main_Window->root.addChild(move(menu));

    // ===== START PAGE =====
    auto start_label = make_unique<Label>("START WINDOW", Position{ 0,0 });
    start_label->setSpecialMode(Color::grn, Label::Vertical);
    start_label->setMode(true, 2);
    auto start_textBox = make_unique<TextBox>(Position{ 10,7 });
    start_Window->root.addChild(move(start_label));
    start_Window->root.addChild(move(start_textBox));

    // ===== OPTIONS PAGE =====
    auto opt_label = make_unique<Label>("OPTIONS WINDOW", Position{ 0,0 });
    opt_label->setMode(true, 2);
    options_Window->root.addChild(move(opt_label));

    catalog.push(move(main_Window));
    catalog.run();
}
