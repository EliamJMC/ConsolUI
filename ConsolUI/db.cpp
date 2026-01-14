/*
	Description : Programme qui permet de choisir un mot au hasard dans une liste de mots provenants d'un fichier texte
				: Utilitaires disponibles pour mettre en majuscule ou enlever les accents
	Version		: 1.0
*/

#include <iostream>
#include <sstream>
#include <conio.h>
#include <windows.h>

#include "db.h"

using namespace std;

DB db;

DB::DB()
{ 
	setcp(1252);

	for (int i = 0; i < 256; ++i) tr[i] = up[i] = i;

	up[(uint8_t)'à'] = 'À';	up[(uint8_t)'â'] = 'Â';	up[(uint8_t)'ä'] = 'Ä';
	up[(uint8_t)'é'] = 'É';	up[(uint8_t)'è'] = 'È';	up[(uint8_t)'ê'] = 'Ê';	up[(uint8_t)'ë'] = 'Ë';
	up[(uint8_t)'ì'] = 'Ì';	up[(uint8_t)'î'] = 'Î';	up[(uint8_t)'ï'] = 'Ï';
	up[(uint8_t)'ò'] = 'Ò';	up[(uint8_t)'ô'] = 'Ô';	up[(uint8_t)'ö'] = 'Ö';
	up[(uint8_t)'ù'] = 'Ù';	up[(uint8_t)'û'] = 'Û';	up[(uint8_t)'ü'] = 'Ü';
	up[(uint8_t)'ç'] = 'Ç';
	up[(uint8_t)'œ'] = 'Œ';

	tr[(uint8_t)'à'] = 'a';	tr[(uint8_t)'â'] = 'a';	tr[(uint8_t)'ä'] = 'a';
	tr[(uint8_t)'À'] = 'A';	tr[(uint8_t)'Â'] = 'A';	tr[(uint8_t)'Ä'] = 'A';
	tr[(uint8_t)'é'] = 'e';	tr[(uint8_t)'è'] = 'e';	tr[(uint8_t)'ê'] = 'e';	tr[(uint8_t)'ë'] = 'e';
	tr[(uint8_t)'É'] = 'E';	tr[(uint8_t)'È'] = 'E';	tr[(uint8_t)'Ê'] = 'E';	tr[(uint8_t)'Ë'] = 'E';
	tr[(uint8_t)'ì'] = 'i';	tr[(uint8_t)'î'] = 'i';	tr[(uint8_t)'ï'] = 'i';
	tr[(uint8_t)'Ì'] = 'I';	tr[(uint8_t)'Î'] = 'I';	tr[(uint8_t)'Ï'] = 'I';
	tr[(uint8_t)'ò'] = 'o';	tr[(uint8_t)'ô'] = 'o';	tr[(uint8_t)'ö'] = 'o';
	tr[(uint8_t)'Ò'] = 'O';	tr[(uint8_t)'Ô'] = 'O';	tr[(uint8_t)'Ö'] = 'O';
	tr[(uint8_t)'ù'] = 'u';	tr[(uint8_t)'û'] = 'u';	tr[(uint8_t)'ü'] = 'u';
	tr[(uint8_t)'Ù'] = 'U';	tr[(uint8_t)'Û'] = 'U';	tr[(uint8_t)'Ü'] = 'U';
	tr[(uint8_t)'ç'] = 'c';
	tr[(uint8_t)'Ç'] = 'C';
}

void DB::setcp(unsigned cp) { SetConsoleCP(cp); SetConsoleOutputCP(cp); }

char DB::upper(char c) { return up[unsigned char(toupper(c))]; }
char DB::noacc(char c) { return tr[unsigned char(c)]; }

string DB::upper(string s)
{
	for (size_t i = 0; i < s.size(); ++i) s[i] = upper(s[i]); return s;
}

string DB::noacc(string s)
{
	for (size_t i = 0; i < s.size(); ++i) s[i] = noacc(s[i]); return s;
}

void DB::showfnm()
{
	cout << "base de données:" << "\n\n\"" << fnm << "\"" << "\n\n";
}

void DB::open(string fnm)
{ 
	f.open(this->fnm = fnm);
	if (!f.is_open()) { system("cls"); cout << "impossible d'ouvrir le fichier:\n\n\"" << fnm + "\""; (void)_getch(); exit(0); }
	else if (size() == 0) { system("cls"); cout << "fichier vide:\n\n\"" << fnm + "\""; (void)_getch(); exit(0); }
}

void DB::close() { f.close(); }

streamoff DB::size() { if (f.is_open()) { f.seekg(0, ios::end); return f.tellg(); } else return 0; }

void DB::clean(string& s)
{
	stringstream ss(s); string w;
	s = "";	ss >> s; while (ss >> w) s += " " + w;
}

void DB::randseek()
{
	if (f.fail() || f.eof()) f.clear();
	f.seekg(streamoff(std::rand()) * streamoff(std::rand()) % size(), ios::beg);					// division par zéro si fichier vide
	while (f.tellg() > 0) if (f.peek() != '\n') f.seekg(-1, ios::cur); else break;
	if (f.tellg() > 0) f.get(); // consomme de '\n'
}

void DB::randline(string& l)
{
	if (f.is_open()) { randseek(); getline(f, l); clean(l); }
}


void DB::randword(string& w, size_t s)
{
	if (f.is_open())
		do { randline(w); } while (w.size() != s);
}

string DB::rand(size_t s)
{
	string w;
	if (s >= 4 and s <= 5) randword(w,s);
	return w;
}

bool DB::exist(string s)
{
	string l;
	streamoff mino, maxo, mido;

	clean(s); s = upper(noacc(s));				// la valeur normalisée s à trouver

	mino = 0, maxo = size() - 1;
	while (mino <= maxo and !f.fail())
	{
		mido = (mino + maxo) / 2;				// le milieu

		f.seekg(mido, ios::beg);
		while (f.tellg() > 0)
		{
			if (f.peek() != '\n')
				f.seekg(--mido, ios::beg);		// reculer jusqu'au \n
			else break;
		}
		if (f.peek() == '\n') { f.get(); ++mido; }

		getline(f,l); clean(l); l = upper(noacc(l));

		if (s == l)			return true;
		else if (s < l)		maxo = mido - 2;
		else				mino = mido + l.size();
	}
	return false;
}