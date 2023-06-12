
void main() {
    // Good: 
    while (false);

    // Bad: no predicate
    while () {
        int x = 0;
    }
}