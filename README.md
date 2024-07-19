# Open Oceans++

C++ implementation of Open Oceans

See [https://github.com/jonm3D/OpenOceans](https://github.com/jonm3D/OpenOceans)

# Dependencies

``` bash
dnf -y install \
    gcc \
    cmake \
    cppcheck \
    python3.12  \
    parallel \
    gmp-devel
```

# Build and test

``` bash
$ make build test
```

# Classify and Score

Create symlink to the input data:

``` bash
$ ln -s <path to data>/data
```

``` bash
$ make classify
...
```

Classifed CSV files will be written to a `predictions` directory. Each
input granule CSV will have a corresponding classified `.csv` file and
a `.txt` file containing performance statistics.

``` bash
$ ls -1 predictions/
ATL03_20230213042035_08341807_006_01_gt2l_0_classified.csv
ATL03_20230328030558_01031906_006_01_gt2l_0_classified.csv
ATL03_20230407113920_02611908_006_01_gt1l_0_classified.csv
ATL03_20230407113920_02611908_006_01_gt2l_0_classified.csv
ATL03_20230407113920_02611908_006_01_gt3l_0_classified.csv
...
```

``` bash
$ make score
Reading filenames from stdin
180 filenames read
...
cls     acc     F1      bal_acc cal_F1  tp      tn      fp      fn      support total
0       0.983   0.867   0.924   0.914   26346   448235  3638    4464    30810   482683
40      0.998   0.952   0.981   0.980   11044   470520  696     423     11467   482683
41      0.985   0.992   0.951   0.913   437142  38460   3817    3264    440406  482683
...
Surface
Average Acc = 0.944
Average F1 = 0.954
Average BA = 0.925
...
Bathy
Average Acc = 0.980
Average F1 = 0.603
Average BA = 0.893
```
