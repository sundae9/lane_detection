import datetime as dt
from glob import glob

#============================== 
# 직접 작성하는 부분
# 측정 항목
item = ["min (ms)", "min-frame-idx", "max (ms)", "max-frame-idx", "avg"]
# 프로세스 이름
process=["1", "2", "3", "4", "5","6","7","8", "9"]

# 저장 경로
DST_PREFIX = "../result/"
file_name = dt.datetime.now().strftime("%Y-%m-%d-%H-%M-%S") + ".csv"

# 출력 결과 txt
SRC = "../result/tmp/test*"
#==============================

def get_content(file_list):
    """
    cpp로 출력한 txt 파일에서 결과 값 가져오는 함수
    """
    content = []

    for file in file_list:
        f = open(file, 'r')
        tmp = f.read().split('\n')[:-1]
        tmp2 = [line.split(',')[:-1] for line in tmp]
        tmp2_t = list(zip(*tmp2))
        content.append(tmp2_t)

        f.close()

    return content

def write_csv(content, file_path, tc):
    """
    csv 작성하는 함수
    """
    file =  open(file_path, 'w')
    file.write(", ," + ','.join(process) + "\n")
    for i in range(len(item)):
        file.write(item[i])
        for j in range(tc):
            file.write(",#{},".format(j+1)+','.join(content[j][i+1])+'\n')
        file.write('\n\n')
    file.close()

    print("file {} saved".format(file_name))
    return

if __name__ == "__main__":
    tc = int(input())
    file_list = sorted(glob(SRC))[:tc]
    print(file_list)
    content = get_content(file_list)
    print(len(content))

    write_csv(content, DST_PREFIX+file_name, tc)