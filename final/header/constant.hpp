#ifndef PARK_CONSTANT_HPP
#define PARK_CONSTANT_HPP

// 영상 정보
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 360
#define FRAME_ROWS FRAME_HEIGHT
#define FRAME_COLS FRAME_WIDTH

// roi 설정 값
#define DX 15 // 2*DX = 동적 ROi 폭
#define DEFAULT_ROI_UP 195 // ROI 상단 y값
#define DEFAULT_ROI_DOWN 284 // ROI 하단 y값
#define DEFAULT_ROI_HEIGHT (DEFAULT_ROI_DOWN - DEFAULT_ROI_UP) // ROI 높이

// 기타 변수
#define GRADIENT_STD 1.73 // cotanent(30 deg) = 1.73

// MODE
#define SHOW //imshow 활성화
#define DEBUG // 출력 문구
#define GRAPHIC

#endif //PARK_CONSTANT_HPP