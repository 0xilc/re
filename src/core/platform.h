#pragma once

#if defined(__CUDACC__) 
  #define RE_HD __host__ __device__
#else
  #define RE_HD
#endif