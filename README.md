This is a simple benchmark to test if there is any performance benefit for using 1D over 2D textures.

## Results
In my computer, I have found that there is a very small differerence. The following table measures seconds

|           | 1D      | 2D      |  Ratio  |   |
|-----------|---------|---------|---------|---|
| GTX 750Ti | 12.2418 | 12.3267 |  +0.69% |   |
|           |         |         |   |   |
|           |         |         |   |   |

If you want to share your measurements, feel free to make a pull request.

## How to run the benchmark

Download this repo.

Compile as any normal CMake project.

```
cd tex1d_benchmark
mkdir build
cd build
cmake ..
make
```

Run the generated `tex1d_benchmark` executable.

It will print something like this to the console (stdout):

```
1D: 12.2418
2D: 12.3267
```