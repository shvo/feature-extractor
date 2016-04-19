kernel void krnl_dist(__global int *a, __global int *b, int n) {
 
   // for ( int j = 0; j < 10; ++j) {
    for (int i = 10; i < n; ++i) {
      //b[i] = b[i-1] + 3; // bug
      b[i] = b[i-2] + 3;
    }
    //}
}
