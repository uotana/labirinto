//
// Created by Uotana on 07/05/23.
//

#include "noh.h"
#include "coord.h"

// Construtores
Noh::Noh(): pos(Coord (0,0)), ant(Coord (0,0)) {}
Noh::Noh(Coord C): pos(C){}

double Noh::f() const
{
    return g+h;
}

double Noh::heuristica(Coord& dest) const{
    double deltaX = abs(this->pos.col - dest.col);
    double deltaY = abs(this->pos.lin - dest.lin);
    return sqrt(2)*fmin(deltaX, deltaY)+abs(deltaX-deltaY);
}

bool Noh::operator==(const Noh& noh) const
{
    return noh.pos==this->pos;
}
