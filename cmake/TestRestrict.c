int foo(int *TESTRESTRICTDEF i, int *TESTRESTRICTDEF j) {
    return *i + *j;
}
int main(void) {
    int i = 0, j = 0;
    return foo(&i,&j);
}
