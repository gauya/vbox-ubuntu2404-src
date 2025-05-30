class Dummy:
    def __enter__(self):
        print("🔓 리소스 열기")
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        print("🔒 리소스 닫기")

with Dummy() as d:
    print("✨ 작업 중")
