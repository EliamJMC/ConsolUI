#pragma once

// nettoyage de la fenêtre
void clreol();								// efface la ligne courante à partir du curseur sans le déplacer
void clreoscr();							// efface la fenêtre à partir du curseur sans le déplacer
void clrscr();								// efface toute la fenêtre et remet le curseur en (0,0)


// positionnement du curseur
size_t wherex();							// retourne la coord x du curseur
size_t wherey();							// retourne la coord y du curseur
void gotoxy(size_t x, size_t y);			// place le curseur à la colonne x et à la ligne y  ( gotoxy(0,0) est le coin supérieur gauche )


// affichage du curseur de la console
void showcurs(bool visible);				// affiche ou cache le curseur


// coloration des caractères
enum class Color
{ 
	blk, blu, grn, aqua, red, pur, yel, wht, gry, _blu, _grn, _aqua, _red, _pur, _yel, _wht
};

void setcolor(Color tx);					// ex: setColor(Color::yel); --> les prochains cout seront désormais en jaunes 
void setcolor(Color bg, Color tx);			// 


// jeu de caractères
void setcp(unsigned cp);					// ex: cp = code page = 1252 pour les accents


// dimension de la fenêtre et de la mémoire tampon
void setwsize(size_t rows, size_t cols);


// nettoyage de l'input de la console
void clrcin();


// éteindre certains warnings de visual studio
#pragma warning (disable:4326)				// warnings C4326:  le type de retour de 'main' doit être 'int' au lieu de 'void'
#pragma warning (disable:6031)				// warnings C4326:  valeur de retour ignorée '_getch'
#pragma warning (disable:26812)				// warnings C26812: le type enum 'type-name' est non délimité. Préférez 'enum class' à 'enum' (enum.3)