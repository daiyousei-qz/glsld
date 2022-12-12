// Testing ';' inference for limited number of constructs

int x /*;*/

struct Point {
    float x /*;*/
    float y /*;*/
} /*;*/

void main() {
    Point p /*;*/
    p.x = 0 /*;*/
    p.y = 0 /*;*/
}