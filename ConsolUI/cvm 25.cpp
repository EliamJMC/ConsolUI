#include <iostream>
#include <conio.h>
#include <windows.h>

#include "cvm 25.h"

using namespace std;

#undef clrscr
#undef clreoscr

// CVM

class CVM
{
	public:

		void clrscr();
		void clreol();
		void clreoscr();

		size_t wherex();
		size_t wherey();
		void gotoxy(size_t x, size_t y);

		void showcurs(bool visible);
		void setcp(unsigned cp);

		void setcolor(Color tx);
		void setcolor(Color bg, Color tx);

		void clrcin();

		CVM();

	private:

		HANDLE h;
		CONSOLE_SCREEN_BUFFER_INFOEX csbi;
};

CVM::CVM()
{
	h = GetStdHandle(STD_OUTPUT_HANDLE);
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
}

void CVM::clreol()
{
	DWORD length, dummy;
	GetConsoleScreenBufferInfoEx(h, &csbi);
	length = csbi.srWindow.Right - csbi.dwCursorPosition.X;
	FillConsoleOutputCharacter(h, 32, length, csbi.dwCursorPosition, &dummy);
}

void CVM::clreoscr()
{
	DWORD length = 0xFFFFFFFF, dummy;
	GetConsoleScreenBufferInfoEx(h, &csbi);
	FillConsoleOutputCharacter(h, 32, length, csbi.dwCursorPosition, &dummy);
}

void CVM::clrscr()
{
	DWORD length = 0xFFFFFFFF, dummy;
	FillConsoleOutputCharacter(h, 32, length, {}, &dummy);
	SetConsoleCursorPosition(h, {});
}

size_t CVM::wherex() { GetConsoleScreenBufferInfoEx(h, &csbi); return csbi.dwCursorPosition.X; }
size_t CVM::wherey() { GetConsoleScreenBufferInfoEx(h, &csbi); return csbi.dwCursorPosition.Y; }

void CVM::gotoxy(size_t x, size_t y) { SetConsoleCursorPosition(h, { short(x), short(y) }); }

void CVM::showcurs(bool visible)
{
	CONSOLE_CURSOR_INFO info;
	GetConsoleCursorInfo(h, &info);
	info.bVisible = visible;
	SetConsoleCursorInfo(h, &info);
}

void CVM::setcp(unsigned cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); }

void CVM::setcolor(Color tx) { SetConsoleTextAttribute(h, WORD(tx)); }
void CVM::setcolor(Color bg, Color tx) { SetConsoleTextAttribute(h, WORD(bg) * 16 + WORD(tx)); }

void CVM::clrcin()
{ 
	cin.clear();
	cin.ignore(cin.rdbuf()->in_avail());
	while (_kbhit()) (void)_getch();
}

// CVM API

static CVM cvm;

void clreol() { cvm.clreol(); }
void clreoscr() { cvm.clreoscr(); }
void clrscr() { cvm.clrscr(); }

size_t wherex() { return cvm.wherex(); }
size_t wherey() { return cvm.wherey(); }
void gotoxy(size_t x, size_t y) { cvm.gotoxy(x, y); }

void showcurs(bool visible) { cvm.showcurs(visible); }
void setcp(unsigned cp) { cvm.setcp(cp); }

void setcolor(Color tx) { cvm.setcolor(tx); };
void setcolor(Color bg, Color tx) { cvm.setcolor(bg, tx); };

void clrcin() { cvm.clrcin(); };



// CONSOLE

class Console
{
	public:

		// buffer size
		size_t getb_rows();
		size_t getb_cols();

		bool setb_rows(size_t rows);
		bool setb_cols(size_t cols);

		// window size
		size_t getw_rows();
		size_t getw_cols();
		size_t getw_maxrows();
		size_t getw_maxcols();

		bool setw_size(size_t rows, size_t cols);
		bool setw_fullscreen();

		// window position
		int getw_x();
		int getw_y();
		bool setw_pos(int x, int y);

		Console();

	private:

		HANDLE h;
		CONSOLE_SCREEN_BUFFER_INFOEX csbi;

		void get_csbi();
		bool set_csbi();
		void setw_rect(SMALL_RECT rect);
};


Console::Console()
{
	h = GetStdHandle(STD_OUTPUT_HANDLE);
	csbi.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
}

void Console::get_csbi() { GetConsoleScreenBufferInfoEx(h, &csbi); }
bool Console::set_csbi() { return SetConsoleScreenBufferInfoEx(h, &csbi); }

void Console::setw_rect(SMALL_RECT rect) { SetConsoleWindowInfo(h, true, &rect); };


size_t Console::getb_rows() { get_csbi(); return csbi.dwSize.Y; }
size_t Console::getb_cols() { get_csbi(); return csbi.dwSize.X; }

size_t Console::getw_rows() { get_csbi(); return csbi.srWindow.Bottom - csbi.srWindow.Top + 1; }
size_t Console::getw_cols() { get_csbi(); return csbi.srWindow.Right - csbi.srWindow.Left + 1; }

size_t Console::getw_maxrows() { COORD largestSize = GetLargestConsoleWindowSize(h); return largestSize.Y; }
size_t Console::getw_maxcols() { COORD largestSize = GetLargestConsoleWindowSize(h); return largestSize.X; }

int Console::getw_x() { RECT rect; GetWindowRect(GetConsoleWindow(), &rect); return rect.left; }
int Console::getw_y() { RECT rect; GetWindowRect(GetConsoleWindow(), &rect); return rect.top; }


bool Console::setb_rows(size_t rows)
{
	get_csbi();
	if (rows < getw_rows()) return false; // redimensionner la fenêtre avant
	csbi.dwSize.Y = short(rows);
	csbi.srWindow.Bottom += 1; // bug the windows ???  
	return set_csbi();
}

bool Console::setb_cols(size_t cols)
{
	get_csbi();
	if (cols < getw_cols())	return false; // redimensionner la fenêtre avant
	csbi.dwSize.X = short(cols);
	csbi.srWindow.Bottom += 1; // bug the windows ???  
	return set_csbi();
}

bool Console::setw_size(size_t rows, size_t cols)
{
	if ((cols == 0) || (rows == 0)) return false; // deuhh ...

	COORD largestSize = GetLargestConsoleWindowSize(h);
	if ((cols > size_t(largestSize.X)) || (rows > size_t(largestSize.Y))) return false; // rentre pas dans l'écran...

	get_csbi();

	if (rows > size_t(csbi.dwSize.Y)) setb_rows(rows);
	if (cols > size_t(csbi.dwSize.X)) setb_cols(cols);

	setw_rect({ 0, 0, short(cols-1), short(rows-1) });
	setb_cols(cols);
	return true;
}

bool Console::setw_pos(int x, int y)
{
	HWND consoleWindow = GetConsoleWindow();
	return SetWindowPos(consoleWindow, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

bool Console::setw_fullscreen()
{
	setw_size(getw_maxcols(), getw_maxrows());
	return setw_pos(-8, 0);
}


// CONSOLE API

static Console cs;

void setwsize(size_t rows, size_t cols) { cs.setw_size(rows, cols); }