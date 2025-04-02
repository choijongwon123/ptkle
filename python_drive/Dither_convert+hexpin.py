import cv2
import os
import numpy as np
from os import listdir
from os.path import isfile, join

def resize_image(image, width, height):
    """
    이미지 크기를 지정된 width, height로 조정합니다.
    이미지의 종횡비(aspect ratio)는 crop을 통해 유지합니다.
    
    Args:
        image: 입력 이미지
        width: 목표 너비
        height: 목표 높이
    
    Returns:
        종횡비를 유지한 채 crop 후 조정된 이미지
    """
    target_aspect = width / height
    h, w = image.shape[:2]
    current_aspect = w / h

    # Crop the image to maintain the aspect ratio
    if (current_aspect > target_aspect):
        new_w = int(h * target_aspect)
        x_offset = (w - new_w) // 2
        crop_img = image[:, x_offset:x_offset+new_w]
    elif (current_aspect < target_aspect):
        new_h = int(w / target_aspect)
        y_offset = (h - new_h) // 2
        crop_img = image[y_offset:y_offset+new_h, :]
    else:
        crop_img = image

    # Resize cropped image to target dimensions
    dim = (width, height)
    resized = cv2.resize(crop_img, dim, interpolation=cv2.INTER_AREA)
    return resized

def dithering_channel(channel, strength=0):
    """
    각 채널에 대해 Floyd–Steinberg error diffusion dithering을 수행하여
    0, 85, 170, 255의 네 단계로 양자화합니다.
    
    Args:
        channel: 원래 0-255 범위의 채널 이미지 (np.uint8)
        strength: 디더링 강도 (0.0~1.0, 기본값 1.0)
                 0.0: 디더링 없음
                 1.0: 기본 Floyd-Steinberg 디더링
    Returns:
        오류 확산이 적용된 양자화 채널 이미지 (np.uint8)
    """
    mapping = np.array([0, 85, 170, 255], dtype=np.float64)
    h, w = channel.shape
    dithered = channel.astype(np.float64).copy()
    
    for i in range(h):
        for j in range(w):
            old_value = dithered[i, j]
            diff = np.abs(mapping - old_value)
            idx = np.argmin(diff)
            new_value = mapping[idx]
            dithered[i, j] = new_value
            error = (old_value - new_value) * strength
            
            # Distribute the error to neighboring pixels with adjusted strength
            if (j + 1 < w):
                dithered[i, j + 1] += error * 7 / 16
            if (i + 1 < h and j > 0):
                dithered[i + 1, j - 1] += error * 3 / 16
            if (i + 1 < h):
                dithered[i + 1, j] += error * 5 / 16
            if (i + 1 < h and j + 1 < w):
                dithered[i + 1, j + 1] += error * 1 / 16
    
    dithered = np.clip(dithered, 0, 255)
    return dithered.astype(np.uint8)

def rgb_to_hex_bits(r, g, b):
    """
    Converts RGB values (0, 85, 170, 255) to 6-bit hex value
    B: bits 5-4, G: bits 3-2, R: bits 1-0
    
    Args:
        r, g, b: RGB values (must be 0, 85, 170, or 255)
    
    Returns:
        hex value from 0x00 to 0x3F
    """
    r_bits = {0: 0, 85: 1, 170: 2, 255: 3}[r]
    g_bits = {0: 0, 85: 1, 170: 2, 255: 3}[g]
    b_bits = {0: 0, 85: 1, 170: 2, 255: 3}[b]
    
    hex_value = (b_bits << 4) | (g_bits << 2) | r_bits
    return hex_value

def convert_image(image_path, output_image_path, output_txt_path, dither_strength=1.0):
    """
    이미지를 6비트 이미지로 변환하고 헥스 값을 텍스트 파일로 저장합니다.
    
    Args:
        image_path: 입력 이미지 경로
        output_image_path: 출력 이미지 경로
        output_txt_path: 출력 텍스트 파일 경로
        dither_strength: 디더링 강도 (0.0~1.0)
    """
    # 이미지 읽기
    img_24bit = cv2.imread(image_path, cv2.IMREAD_COLOR)
    if img_24bit is None:
        raise FileNotFoundError(f"이미지 파일을 읽을 수 없습니다: {image_path}")

    # 이미지 크기 조정
    img_resized = resize_image(img_24bit, 64, 48)

    print(f"Original size: {img_24bit.shape[1]}x{img_24bit.shape[0]}")
    print(f"Resized size: {img_resized.shape[1]}x{img_resized.shape[0]}")

    # 디더링 적용 (강도 파라미터 추가)
    img_d_b = dithering_channel(img_resized[:, :, 0], dither_strength)
    img_d_g = dithering_channel(img_resized[:, :, 1], dither_strength)
    img_d_r = dithering_channel(img_resized[:, :, 2], dither_strength)

    # 각 채널을 허용된 값으로 양자화
    allowed_values = np.array([0, 85, 170, 255], dtype=np.uint8)
    for channel in [img_d_b, img_d_g, img_d_r]:
        for i in range(channel.shape[0]):
            for j in range(channel.shape[1]):
                channel[i,j] = allowed_values[np.argmin(np.abs(allowed_values - channel[i,j]))]

    # 채널 병합 (BGR 순서)
    img_6bit = cv2.merge((img_d_b, img_d_g, img_d_r))

    # 이미지 저장 - PNG 형식으로 저장하여 색상 정보 손실 방지
    print(f"Final image size: {img_6bit.shape[1]}x{img_6bit.shape[0]}")
    cv2.imwrite(output_image_path.replace('.jpg', '.png'), img_6bit)
    print(f"이미지 변환 완료: {output_image_path}")

    # 텍스트 파일로 헥스 값 저장
    image_rgb = cv2.cvtColor(img_6bit, cv2.COLOR_BGR2RGB)
    h, w, _ = image_rgb.shape

    with open(output_txt_path, "w") as f:
        for i in range(h):
            row_values = []
            for j in range(w):
                r, g, b = image_rgb[i, j]
                hex_value = rgb_to_hex_bits(r, g, b)
                row_values.append(f"0x{hex_value:02X}")
            row_str = "{" + ", ".join(row_values) + "}"
            f.write(row_str + ",\n")
    
    print(f"이미지 헥스 데이터 저장 완료: {output_txt_path}")

def main():
    # 현재 스크립트 위치를 기준으로 경로 설정
    base_path = os.path.dirname(os.path.abspath(__file__))
    input_path = os.path.join(base_path, "image")
    output_path = os.path.join(base_path, "output")

    # 출력 폴더가 없으면 생성
    if (not os.path.exists(output_path)):
        os.makedirs(output_path)
        print(f"출력 폴더 생성: {output_path}")

    # 입력 폴더 확인
    if (not os.path.exists(input_path)):
        print(f"입력 폴더 '{input_path}'를 찾을 수 없습니다.")
        print(f"스크립트 위치에 'image' 폴더를 생성하세요: {base_path}")
        exit(1)

    # 입력 폴더에서 파일 목록 가져오기
    inputs = [os.path.join(input_path, f) for f in listdir(input_path) if isfile(join(input_path, f))]
    print(f"입력 파일 {len(inputs)}개 발견")

    # 디더링 강도 설정
    dither_strength = 0.7  # 기본값으로 0.7 사용 (0.0~1.0)
    
    # 각 파일 처리
    for path in inputs:
        base_filename = os.path.splitext(os.path.basename(path))[0]
        output_image_path = os.path.join(output_path, f"{base_filename}.png")  # jpg 대신 png 사용
        output_txt_path = os.path.join(output_path, f"{base_filename}.txt")
        
        print(f"\n처리 중: {path}")
        try:
            convert_image(path, output_image_path, output_txt_path, dither_strength)
        except Exception as e:
            print(f"파일 처리 중 오류 발생: {e}")

if __name__ == "__main__":
    main()
