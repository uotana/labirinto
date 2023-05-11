//
// Created by Uotana on 07/05/23.
//

#ifndef LABIRINTO_NOH_H
#define LABIRINTO_NOH_H
#include "coord.h"

struct Noh {
    public:
    Coord pos, ant;
    double g, h;

    Noh();
    Noh(Coord C);
    double f() const;
    double heuristica(Coord& dest) const;
    bool operator==(const Noh& noh) const;
    bool operator<(const Noh& noh) const;
};


#endif //LABIRINTO_NOH_H
