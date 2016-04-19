unsigned two_indep_loop(int *a, int *b, int *c, int *d, int *e, int *f) {
    for ( unsigned i = 0; i < 10; i++ ) {
        a[i] = b[i] + c[i];
    }

    for ( unsigned j = 0; j < 10; j++ ) {
        f[j] = d[j] + e[j];
    }

    return 0;
}
