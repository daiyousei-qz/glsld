void foo(double x, float y) {

}

void foo(float x, double y) {

}

void bar() {
    // Error: ambiguous call
    foo(float(1.0), 1.);
}

struct S {
    float x;
};

void main() {
    /* type name */ S /* variable name */ S;
    /* variable name */ S;

    // constructor call
    S[2]();
}