import qrcode

def generate_qr_code(data, file_name):
  qr = qrcode.QRCode(
    version=1, # QR 코드 크기 (1~40, 자동은 None)
    error_correction=qrcode.constants.ERROR_CORRECT_M,
    box_size=10, # 각 점의 크기 (픽셀 단위)
    border=4, # 테두리 여백 (기본값 4)
    )
  qr.add_data(data)
  qr.make(fit=True)
  img = qr.make_image(fill_color="black", back_color="white")
  img.save(file_name)

# QR에 넣을 데이터
dat = "https://www.example.com"

generate_qr_code(dat,"qr-image.png")

"""
import qrcode
img = qrcode.make("https://www.example.com")
img.save("simple_qr.png")
"""
