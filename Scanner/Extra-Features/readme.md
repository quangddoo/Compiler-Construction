### To compile
```
make clean
make
```
### Available options
* Display options
```
./scanner --help
```
* Scanner options
  * **1.** Allow underscore **_** as letter
  * **2.** Underscore **_** is considered a special character
  * **4.** **<>** and **><** are considered as non-equivalent symbols
  * **8.** Allow string constants sperated by double qoute **"**
  * **16.** Allow single qoute **'** for character
  * **32.** String length up to 255 characters (only compare the first 15)
* Examples
```
# Enable options 2 and 4 by passing the third parameter as 6 (= 2 + 4)
./scanner example.kpl 6

# To enable all features (63 = 1 + 2 + 4 + 8 + 16 + 32)
./scanner feature/example1.kpl 63

# To run original example
./scanner test/example1.kpl 0
```