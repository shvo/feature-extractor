//kernel void krnl_dist(__global int *a, __global int *b, int n) {
kernel void krnl_dist(__global float *a, __global float *b, int n) {
 
    for (int i = 10; i < n; ++i) {
      b[i] = b[i-2] + 3;
    }
}
