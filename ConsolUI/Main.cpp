#include <iostream>
#include <iomanip>
#include <conio.h>
#include <cstdlib>
#include <string>
#include <functional>
#include <vector>
#include <initializer_list>
#include <map>
#include <stack>
#include <memory>
#include <optional>
#include <algorithm>

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
class Selector;
class Page;
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
    * @param child Any Object That Heritates Form Component
    */
    void addChild(unique_ptr<Component>&& child) {
        child->setParent(this);
        children.push_back(move(child));
    }

    /* Renders every child component on screen.*/
    virtual void render() override {
        for (const auto& child : children) child->render();
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

        if (spaced && !text.empty()) {
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
    void setMode(bool _centered = false, bool _spaced = false, int _spacement = 0) {
        centered = _centered; spaced = _spaced; spacement = _spacement;
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
    bool spaced = false;
    int spacement = 0;
};

//---------------------//
//-- CLASS : TEXTBOX --//
//---------------------//

class TextBox : public Component {
public: 
    TextBox(Position p = { 0,0 }, Size s = { 3,7 }, Action act = nullptr)
        : Component(p, s), action(act) {}

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

    void handle_Input() {
        optional<char> ch = nullopt;
        gotoxy(static_cast<size_t>(absolute_Position().x + 1), static_cast<size_t>(absolute_Position().y + size.height / 2));
        while (ch != 13) {
            ch = verify_character();

            if (ch == 8) {
                if (!word().empty()) {
                    word().pop_back();
                    cout << " \b";
                }
            }
            else if (ch.has_value()) {
                word().push_back(ch.value());
                cout << ch.value(); 
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
    string word() {
        if (word().length() > size.width - 2)
            return word().substr(0, size.width - 2);
        else return word();
    }
};

//----------------------//
//-- CLASS : SELECTOR --//
//----------------------//

class Selector : public Component {
public:

    Selector(Position p = { 0,0 }, Size s = { 30,120 })
        : Component(p, s) {
    }

    /* Renders every child (Label) component on screen.*/
    void render() override {
        for (auto& item : items)
            item.label->render();
    }

    /* 
    * Switch between the Labels in the selector.
    * @param key The letter that set the direction.
    */
    void switch_Selection(char key) {
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
      
private:
    struct Item {
        Label* label;
        Action next;
        optional<Action> back;
    };

    vector<Item> items;
    size_t selectedIndex = 0;
};

//------------------//
//-- CLASS : PAGE --//
//------------------//

class Page {
protected:
    string name;
    Selector* selector = nullptr;

public:
    Page(string _name)
        : root({ 0,0 }, { 30,120 }), name(_name) {}

    Conteiner root;
    bool open = false;

    /* Set the Selector. */
    void set_Selector(Selector* sel) { selector = sel; }

    void handle_Input(char key, Catalog& catalog);

    void read_Page() {
        root.render();
    }
};

//---------------------//
//-- CLASS : CATALOG --//
//---------------------//

class Catalog {
public:
    void run() {
        redraw();
        while (!pageStack.empty()) {
            char key = _getch();
            current()->handle_Input(key, *this);
        }
    }

    void push(Page* page) {
        if (!pageStack.empty())
            pageStack.top()->open = false;

        page->open = true;
        pageStack.push(page);
        redraw();
    }

    void pop() {
        if (pageStack.size() <= 1) {
            exit(0);
        }

        pageStack.top()->open = false;
        pageStack.pop();
        pageStack.top()->open = true;
        redraw();
    }

    Page* current() {
        return pageStack.empty() ? nullptr : pageStack.top();
    }

private:
    void redraw() {
        clrscr();
        if (current()) current()->read_Page();
    }

    stack<Page*> pageStack;
};

// Implementation del metodo de page
void Page::handle_Input(char key, Catalog& catalog) {
    if (key == 27) { catalog.pop(); return; }
    if (selector) selector->switch_Selection(key);
}

//------------------------------//
//-- MAIN : PROGRAM EXECUTION --//
//------------------------------//

int main() {
    Catalog catalog;

    Page main_Page("Main Page");
    Page start_Page("Start Page");
    Page options_Page("Options Page");

    // ===== MAIN PAGE =====
    auto title = make_unique<Label>("MAIN PAGE", Position { 0,0 });
    title->setMode(true, true, 2); 

    auto menu = make_unique<Selector>(Position { 0,4 });

    auto l_start = make_unique<Label>("Start", Position { 0,0 });
    auto l_options = make_unique<Label>("Options", Position { 0,3 });
    auto l_exit = make_unique<Label>("Exit", Position { 0,6 });

    menu->addItem(l_start.get(),    [&]() { catalog.push(&start_Page); });
    menu->addItem(l_options.get(),  [&]() { catalog.push(&options_Page); });
    menu->addItem(l_exit.get(),     [&]() { exit(0); });
    Selector* menu_ptr = menu.get();

    main_Page.root.addChild(move(title));
    main_Page.root.addChild(move(menu));
    main_Page.set_Selector(menu_ptr);

    // ===== START PAGE =====
    auto start_label = make_unique<Label>("START PAGE", Position { 0,0 });
    start_label->setSpecialMode(Color::grn, Label::Vertical);
    start_label->setMode(true, true, 2);
    start_Page.root.addChild(move(start_label));

    // ===== OPTIONS PAGE =====
    auto opt_label = make_unique<Label>("OPTIONS PAGE", Position { 0,0 });
    opt_label->setMode(true, true, 2);
    options_Page.root.addChild(move(opt_label));

    catalog.push(&main_Page);
    catalog.run();
}
