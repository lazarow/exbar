# exbar

Implementacja algorytmu `exbar` opisanego w artykule __Faster Algorithms for
Finding Minimal Consistent DFAs__.

Program powinien bez większych problemów skompilować za pomocą G++ lub MinGW. W zależności od argumentów przekazanych do kompilatora program będzie wyświetlał pełne lub uproszczone logi.

Uwaga! Niniejsza implementacja operuje tylko na liczbach (czy inaczej indeksach znaków).

## Kompilacja

Kompilacja z pełnymi logami:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11
```

Kompilacja z uproszczonymi logami:
```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11 -DELPP_DISABLE_DEBUG_LOGS
```

## Uruchomienie

```
exbar --file {ścieżka do pliku wejściowego} [--verify]
```
Dodanie parametru `--verify` uruchomi weryfikację wygenerowanego zminimalizowanego DFA za pomocą przykładów i kontrprzykładów z pliku wejściowego.

## Format pliku wejściowego

```
{liczba przykładów} {rozmiar słownika}
{pojedynczy przykład: {akceptowany/nieakceptowany} {liczba znaków w słowie} {indeksy znaków oddzielone spacją}}
```

Przykład 1:
```
5 2
1 1 1
1 4 1 0 1 1
1 2 0 0
0 3 1 0 0
0 1 0
```

Przykład 2:
```
8 2
1 11 0 0 1 0 0 0 0 0 1 0 0
0 15 0 0 0 0 0 1 1 1 0 1 0 0 0 1 1
1 12 1 1 0 0 1 1 0 1 0 1 0 0
0 14 0 1 1 1 1 1 0 1 1 1 0 0 1 0
0 14 0 0 0 1 0 0 0 1 1 0 0 0 1 1
1 15 0 1 0 0 0 1 0 1 1 1 1 0 0 0 1
1 15 0 0 0 1 0 1 1 0 1 1 0 0 0 1 0
0 15 1 1 0 0 1 1 0 1 1 1 1 0 0 1 0
```

## Format pliku wyściowego

Program generuje zminimalizowany DFA do dwóch plików wyjściowych różniących się formatem - pliki nazywają się `dfa.txt` oraz `dfa.yaml`.

### Pierwszy format (txt)

```
{liczba stanów}
{rozmiar alfabetu}
{liczba stanów akceptujących stany akceptujące}
{liczba stanów nieakceptujących stany nieakceptujące}
{przejścia dla stanu 0 w kolejności indeksów znaków alfabetu}
{przejścia dla stanu 1 w kolejności indeksów znaków alfabetu}
...
```

Przykład:
```
3
2
2 0 1
1 2
1 1
0 2
2 0
```

### Pierwszy format (yaml)

Przykład:
```
number of states: 3
size of alphabet: 2
accepting states: [0, 1]
rejecting states: [2]
initial state: 0
transitions:
- [0, 1, 1]
- [1, 0, 2]
- [2, 2, 0]
```

## Szybki start

```
g++ exbar.cpp libraries/easylogging/easylogging++.cc -std=c++11 -DELPP_DISABLE_DEBUG_LOGS && ./a.out --file samples/sample1.txt
```