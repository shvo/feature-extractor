//kernel void krnl_dist(__global int *a, __global int *b, int n) {
kernel void krnl_dist(__global float *a, __global float *b, int n) {
  int c;
  for (int j = 10; j < n; ++j) {
    c = j + 2;
    for (int i = 10; i < n; ++i) {
      b[i] = b[i-2] + 3;
    }
  }
}
