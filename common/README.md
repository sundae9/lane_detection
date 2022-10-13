### 공통으로 사용하는 소스코드

1. timer.cpp

- 시간 측정 및 측정 값 출력하는 모듈<br/>

  ```cpp
  // 예시
  #include <../common/timer.cpp>

  Video_info vi;
  Initialize_Video_info(vi);

  TickMeter tm;
  tm.reset();
  tm.start();

  // do something
  Time_record(tm, vi, 0);
  ```

2. make_csv.py

- SRC에 있는 txt 파일을 읽어서 csv 파일을 작성 (제목: `YYYY-MM-DD-HH-MM-SS.csv`)

3. test.sh

- 횟수를 입력 받아서 실행 파일(./main)의 실행 결과를 result/tmp에 txt로 작성, 이후 make_csv.py 실행
- test.sh의 상단 폴더에 result/tmp가 존재해야 함
- tmp 내부에 저장되는 testX.txt 파일은 매번 새로 덮어쓰기 된다. 원본 필요하면 따로 백업할 것.

```bash
# ex)
./test.sh 10
```

폴더구조 예시

```
park
├── CMakeLists.txt
├── build
│   ├── CMake 관련
│   ├── 실행파일
│   ├── make_csv.py
│   └── test.sh
├── result
│   ├── 2022-10-09-10-58-02.csv
│   └── tmp
│       ├── test0.txt
│       ├── ...
│       └── test9.txt
└── main.cpp
```
