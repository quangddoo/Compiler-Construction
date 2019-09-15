### To compile
```
make clean
make
```
### To run example
```
./scanner test/example1.kpl
```
### Compare with the provided result
```
./scanner test/example1.kpt > result1.txt
diff result1.txt test/result1.txt
```