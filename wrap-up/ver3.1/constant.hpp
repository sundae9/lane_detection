#ifndef VER3_1_CONSTANT_HPP
#define VER3_1_CONSTANT_HPP

// 영상 정보
#define LABEL_OFFSET 10
#define FRAME_WIDTH 640
#define FRAME_HEIGHT 285
#define FRAME_ROWS FRAME_HEIGHT
#define FRAME_COLS FRAME_WIDTH

// roi 설정 값
#define DX 15 // 2*DX = 동적 ROi 폭
#define DEFAULT_ROI_LEFT 89 // ROI 좌측 x값
#define DEFAULT_ROI_RIGHT 529 // ROI 우측 x값
#define DEFAULT_ROI_UP 185 // ROI 상단 y값
#define DEFAULT_ROI_DOWN 274 // ROI 하단 y값
#define DEFAULT_ROI_CENTER 309
#define DEFAULT_ROI_HEIGHT (DEFAULT_ROI_DOWN - DEFAULT_ROI_UP) // ROI 높이
#define DEFAULT_ROI_WIDTH (DEFAULT_ROI_RIGHT - DEFAULT_ROI_LEFT)
// 기타 변수
#define GRADIENT_DOWN_STD 2 // cotangent(26.56 deg) = 2
#define GRADIENT_UP_STD 0.176327 // cotangent(80 deg) = 0.0112...
#define tangent_weight 100 // tangent 값의 화면 표시 비율

#define DEFAULT_BINARIZATION_THRESH 150
#define BORDERLINE_OFFSET 2

#define HOUGH_PARAM1 30
#define HOUGH_PARAM2 20
#define HOUGH_PARAM3 40

// MODE
#define SHOW //imshow  활성화
//#define TIME_TEST // 출력 문구
#define GRAPHIC
//#define VIDEO_SAVE
//#define DETECTION_RATE
#define THRESH_DEBUG
//#define ROI_STAT



#endif //VER3_1_CONSTANT_HPP
