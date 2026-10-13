// Commandes recues du dashboard (via le bridge) sur le port serie.

#ifndef COMMAND_H
#define COMMAND_H

// Lit les commandes JSON recues sur le port serie et applique les seuils.
// Format : une ligne par commande, par exemple
//   {"ht":27}  ou  {"lt":24}  ou  {"lt":24,"ht":27}
// A appeler a chaque tour de loop() : la lecture ne bloque pas.
void command_process();

#endif
