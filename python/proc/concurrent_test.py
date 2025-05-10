import concurrent.futures
import time

def task(n):
    """간단한 작업을 수행하는 함수"""
    print(f"Task {n} 시작")
    time.sleep(n)  # 작업 시간은 n초
    print(f"Task {n} 완료")
    return n * 2

if __name__ == "__main__":
    start_time = time.time()
    numbers = [5, 1, 3, 2, 4]

    # 최대 3개의 스레드를 사용하는 스레드 풀 생성
    with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
        # 각 숫자에 대해 작업을 스레드 풀에 제출하고 Future 객체를 얻음
        futures = {executor.submit(task, num): num for num in numbers}

        # 작업 완료 순서대로 결과 처리
        print("작업 결과:")
        for future in concurrent.futures.as_completed(futures):
            original_number = futures[future]
            try:
                result = future.result()
                print(f"Task {original_number}의 결과: {result}")
            except Exception as e:
                print(f"Task {original_number} 실행 중 오류 발생: {e}")

    end_time = time.time()
    print(f"총 실행 시간: {end_time - start_time:.2f} 초")
