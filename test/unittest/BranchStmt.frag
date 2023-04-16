
void main() {
    // Good: one branch
    if (true) {
    }

    // Good: two branch
    if (true) {
    }
    else {
    }

    // Bad: no predicate
    if () {
    }

    // Bad: no predicate
    if {
    }
}