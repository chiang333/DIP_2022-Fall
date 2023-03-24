# HW1

## 1. Flip Img
### COMPILE
```
g++ -std=c++14 flip.cpp -o flip
```

### EXECUTE
```
./flip input1.bmp input2.bmp
```

### OUTPUT FILE
./`output1.bmp` `output2.bmp`

---

## 2. Resolution
### COMPILE
```
g++ -std=c++14 resolution.cpp -o resolution
```

### EXECUTE
```
./resolution input1.bmp input2.bmp
```

### OUTPUT FILE
- `output1_1.bmp`(6 bits)  `output1_2.bmp`(4 bits) `output1_2.bmp`(2 bits)
- `output2_1.bmp`(6 bits) `output2_2.bmp`(4 bits) `output2_2.bmp`(2 bits)

---

## 3. Scaling
### COMPILE
```
g++ -std=c++14 scaling.cpp -o scaling
```

### EXECUTE
```
./scaling input1.bmp input2.bmp
```

### OUTPUT FILE
./`output1_up.bmp` `output1_down.bmp`  
./`output2_up.bmp` `output2_down.bmp`  