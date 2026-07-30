## Design Choices
1. Spectrum with RGB
  I use Spectrum but using it as merely RGB for now, so the engine will be an RGB engine,
  but it's possible to support spectral rendering too.

2. Texture LOD
  I am choosing Ray Cones instead of Ray Differentials, since it's simpler and 2 or 3x faster compared
  to Ray Differentials. 
  See: TEXTURE LEVEL OF DETAIL STRATEGIES FOR REAL-TIME RAY TRACING [https://media.contentapi.ea.com/content/dam/ea/seed/presentations/barrebrisebois2019-texture-level-of-detail-strategies-for-real-time-ray-tracing.pdf][NVIDIA]

3. Self Intersection
  I am using Robust Ray Offset method(Watcher-Binder, Ray Tracing Gems), it's easy to implement, and works error prone.

4. Coordinate System
  glTF-local, so Y-up and right hand, meter convention. I am supporing glTF import, so it makes sense to comply with glTF.

5. Film Accumulation Threading
  Using tile based strategy, each tile is belong to one thread.

6. Primitives Alignment
  Read: [https://leimao.github.io/blog/CUDA-Data-Alignment/]
