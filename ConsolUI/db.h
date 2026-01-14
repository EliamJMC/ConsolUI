/*
	Description : utilitaires pour accéder à une liste de mots triés et enregistrés dans un fichier, un par ligne
				: - rand() pour obtenir une mot au hasard
				: - exist() pour vérifier si un mot est présent dans le fichier
				: - d'autres utilitaires disponibles pour mettre les mots en majuscule ou enlever les accents

	Version		: 1.0
*/

#pragma once

#include <string>
#include <fstream>

using namespace std;

class DB
{
	public:

		void open(string fnm);
		void close();

		string rand(size_t s = 5);

		char upper(char c);
		char noacc(char c);

		string upper(string s);
		string noacc(string s);

		bool exist(string s);

		DB();

	protected:

		void setcp(unsigned cp);

		char up[256];
		char tr[256];

		void clean(string& s);
		void randseek();
		void randline(string& l);
		void randword(string& w, size_t s);

		string fnm;
		void showfnm();

		streamoff size();
		fstream f;
};

extern DB db;