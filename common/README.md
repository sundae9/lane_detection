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
