
void main() {
    // Good: variable decl
    int x;
    int y, z;
    int a = 0, b = 1;

    // Good: struct decl
    struct S1 {
        float x;
    };
    S1 s1;
    s1.x;

    struct S2 { float x; } s2;
    s2.x;

    // Bad: function decl is not allowed
    int foo();
    int bar() {
        return 0;
    }
}