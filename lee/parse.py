import numpy as np

def main():
    # n번째 처리, 최소처리 시간, 최소일때의 프레임, 최대 시간, 최대일때 프레임, 평균처리 시간
    buf = '''0,2.00129,1507,7.72436,96,2.58179,
1,4.15311,1505,10.0267,96,5.24107,
2,4.30706,1505,10.1609,96,5.40087,
3,6.49703,1505,17.3396,409,8.27506,
4,16.038,1045,38.3138,1441,20.4367,
5,16.052,1045,38.3637,1441,20.4589,
6,16.0583,1045,38.3739,1441,20.4687,
7,16.9985,1045,39.3622,1441,21.572,'''

    arrs = buf.split('\n')

    new_buf = []
    
    for arr in arrs:
        new_buf.append(arr.split(',')[:-1])

    np_arr = np.array(new_buf)
    np_arr_t = np_arr.T
    last_buf = np_arr_t.tolist()
    print(last_buf)
    s = '1채널 변환, 이진화, 관심영역 설정, 엣지검출, 직선검출, 가로선제거, 중앙선침범제거, 선 그리기\n'
    
    with open('test.csv', 'w') as f:
        f.write(s)
        for a in last_buf:
            for e in a:
                f.write(e+',')
            f.write('\n')

        

if __name__ == "__main__":
    main()