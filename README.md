Kompilacja z pełnymi logami:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11
```

Kompilacja z uproszczonymi logami:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11 -DELPP_DISABLE_DEBUG_LOGS
```

UWAGA! Biblioteka nie jest skończona!
