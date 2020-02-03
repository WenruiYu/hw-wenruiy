#include <stdexcept>
#include <cmath>
#include <cstdlib>
#include "rational.h"

Rational::Rational()
{
    n = 0;
    d = 1;
}
Rational::Rational(int num, int denom)
{
    if (denom == 0) throw std::invalid_argument("Can't have denom = 0");
    n = abs(num);
    d = abs(denom);
    int sign = 1;
    if(num == 0) {
        normalize0();
    } else if(num*denom < 0) {
        sign = -1;
    }
    reduce();
    n *= sign;
}

void Rational::normalize0()
{
    if (n == 0 && d != 1) d = 1;
}

std::ostream &operator<<(std::ostream &ostr, const Rational &r) {
    ostr << r.n <<"/" << r.d;
    return ostr;
}

std::istream &operator>>(std::istream &istr, Rational &r) {
    std::string t;
    istr >> t;
    int index = t.find_first_of('/');
    if(index >= 0 && index < t.size()-1) {
        r.n = atoi(t.substr(0,index).c_str());
        r.d = atoi(t.substr(index+1).c_str());
    } else {
        if(index == t.size()-1) {
            r.n = atoi(t.substr(0,index).c_str());
            istr >> t;
            r.d = atoi(t.c_str());
        } else {
            r.n = atoi(t.c_str());
            istr >> t;
            if(t.size() == 1) {
                istr >> t;
                r.d = atoi(t.c_str());
            } else {
                r.d = atoi(t.substr(1).c_str());
            }
        }
    }
    r = Rational(r.n,r.d);
    return istr;
}

void Rational::reduce() {
    int x = gcd(n,d);
    n /= x;
    d /= x;
}

int Rational::gcd(int a, int b) {
    return b == 0 ? a : gcd(b, a % b);
}

int Rational::lcm(int a, int b) {
    return (int)((long long) a * b / gcd(a,b));
}

Rational Rational::operator+(const Rational &r) {
    int denom = lcm(r.d,d);
    int n1 = n*denom/d;
    int n2 = r.n*denom/r.d;
    return Rational(n1+n2,denom);
}

Rational Rational::operator+(int x) {
    return Rational(x*d+n,d);
}

Rational operator+(int x, Rational &r) {
    return Rational(x*r.d+r.n,r.d);
}

Rational Rational::operator*(const Rational &r) {
    return Rational(r.n*n,r.d*d);
}

Rational Rational::operator*(int x) {
    return Rational(x*n,d);
}

Rational Rational::operator^(int x) {
    if( x == 0) return Rational(1,1);
    else if(x > 0) return Rational((int)pow(n,x),(int)pow(d,x));
    else {
        return Rational((int)pow(d,-x),(int)pow(n,-x));
    }
}


bool Rational::operator==(const Rational &r) {
    return r.n == n && r.d == d;
}

bool Rational::operator!=(const Rational &r) {
    return r.n != n || r.d != d;
}

Rational operator*(int x, Rational &r) {
    return Rational(x*r.n,r.d);
}

bool Rational::operator<(const Rational &r) {
    int denom = lcm(r.d,d);
    int n1 = n*denom/d;
    int n2 = r.n*denom/r.d;
    return n1 < n2;
}

Rational Rational::operator+=(const Rational &r) {
    return *this+r;
}

Rational Rational::operator*=(const Rational &r) {
    return *this*r;
}

Rational Rational::operator+=(int x) {
    return *this+x;
}

Rational Rational::operator*=(int x) {
    return *this*x;
}


