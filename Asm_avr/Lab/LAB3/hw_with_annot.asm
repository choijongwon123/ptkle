.INCLUDE "m128def.inc"        ; ATmega128의 레지스터 및 관련 정의를 포함

; 레지스터 별칭 정의
.DEF ONES_DIGIT = R20         ; R20: 1의 자리 숫자 데이터 저장
.DEF TENS_DIGIT = R21         ; R21: 10의 자리 숫자 데이터 저장
.DEF temp = R22               ; R22: 계산 및 임시 값 저장용
.DEF COUNT = R23              ; R23: 카운터 변수

.CSEG                        ; 코드 세그먼트 시작
.ORG 0x0000                 ; 프로그램 시작 주소 0x0000

    ; 스택 포인터 초기화
    LDI R16, LOW(RAMEND)     ; RAM끝 주소의 하위 바이트를 R16에 로드
    OUT SPL, R16             ; SPL(스택 포인터 하위 바이트)에 출력
    LDI R16, HIGH(RAMEND)    ; RAM끝 주소의 상위 바이트를 R16에 로드
    OUT SPH, R16             ; SPH(스택 포인터 상위 바이트)에 출력

    ; PORTF를 출력으로 설정 (하위 4비트만 사용)
    LDI R16, 0x0F            ; 0x0F = 0000 1111b, 하위 4비트만 1로 설정
    STS DDRF, R16            ; DDRF에 저장하여 PORTF의 하위 4비트를 출력으로 지정

    ; PORTE를 전부 출력으로 설정
    LDI R16, 0xFF            ; 0xFF = 1111 1111b, 모든 비트를 1로 설정
    OUT DDRE, R16            ; DDRE에 출력하여 PORTE를 출력 전용으로 설정

    LDI COUNT, 0             ; COUNT를 0으로 초기화

LOOP:
    MOV ONES_DIGIT, COUNT    ; COUNT 값을 ONES_DIGIT에 복사 (초기 일의 자리 값)
    CALL DISPLAY_TWO_DIGITS  ; 두 자리 디스플레이 출력 서브루틴 호출

    INC COUNT                ; COUNT 값을 1 증가
    CPI COUNT, 60            ; COUNT와 60 비교 (60까지 카운팅)
    BRNE SKIP              ; COUNT가 60이 아니면 SKIP으로 분기
    LDI COUNT, 0             ; COUNT가 60이면 0으로 리셋

SKIP:
    CALL D100MS              ; 100ms 지연 서브루틴 호출 (멀티플렉싱 사이 딜레이)
    JMP LOOP                 ; 메인 루프로 복귀하여 반복

DISPLAY_TWO_DIGITS:          ; 두 자리 디스플레이 출력 서브루틴
    CALL DIGITS              ; 숫자를 분리하여 TENS_DIGIT와 ONES_DIGIT에 분배

    LDI R17, 0xEF            ; 10의 자리 선택 패턴 (active-low: 0xEF = 1110 1111b)
    OUT PORTE, R17           ; PORTE에 출력하여 10의 자리 디스플레이 활성화
    STS PORTF, TENS_DIGIT    ; PORTF에 TENS_DIGIT 값을 출력 (10의 자리 데이터 전송)

    CALL D100MS              ; 약간의 지연 (10의 자리 표시 유지)

    LDI R17, 0xDF            ; 1의 자리 선택 패턴 (active-low: 0xDF = 1101 1111b)
    OUT PORTE, R17           ; PORTE에 출력하여 1의 자리 디스플레이 활성화
    STS PORTF, ONES_DIGIT    ; PORTF에 ONES_DIGIT 값을 출력 (1의 자리 데이터 전송)
    RET                      ; 서브루틴 종료, 복귀

DIGITS:                      ; 숫자 분리 서브루틴 (일의 자리를 10으로 나눈 몫과 나머지 계산)
    PUSH temp                ; temp 레지스터 값 백업 (스택에 저장)
    LDI TENS_DIGIT, 0        ; TENS_DIGIT(10의 자리 데이터)를 0으로 초기화
    LDI temp, 10             ; 제수 10을 temp에 로드

REPEAT:                      ; 나눗셈 루프: 일의 자리에서 10씩 빼면서 몫 계산
    SUB ONES_DIGIT, temp     ; ONES_DIGIT에서 10을 뺌
    BRMI MINUS               ; 결과가 음수이면 MINUS로 분기 (더 이상 10을 뺄 수 없는 상태)
    INC TENS_DIGIT           ; 결과가 0 이상이면, 10의 자리 값 1 증가
    RJMP REPEAT              ; 루프 반복

MINUS:
    ADD ONES_DIGIT, temp     ; 마지막으로 뺀 10을 더해 복원 (음수 결과 취소)
    POP temp                 ; 백업했던 temp 레지스터 복원
    RET                      ; DIGITS 서브루틴 복귀

; (이후 D500MS, D100MS, D200MS, D200US 등의 지연 서브루틴이 이어짐)

	; DELAY SUBROUTINE
; DELAY SUBROUTINE
D500MS:                     ; 500 밀리초 지연을 위한 서브루틴
    RCALL D100MS           ; 100ms 지연 서브루틴 호출
    RCALL D200MS           ; 200ms 지연 서브루틴 호출
    RCALL D200MS           ; 다시 200ms 지연 서브루틴 호출
    RET                    ; 500ms 지연 후 서브루틴 복귀

D100MS:                     ; 약 100 밀리초 지연을 위한 서브루틴
    LDI R18, 100           ; R18에 100을 로드 (100회 반복용 카운터)
BASE1MS:
    RCALL D200US           ; 200 마이크로초 지연 서브루틴 호출
    RCALL D200US           ; 총 10번 호출하여 약 2ms (200us×10)=2ms 지연
    RCALL D200US
    RCALL D200US
    RCALL D200US
    DEC R18                ; R18 카운터 감소
    BRNE BASE1MS           ; R18가 0이 아니라면, BASE1MS 루프로 반복
    RET                    ; 100ms 지연이 완료된 후 서브루틴 복귀

D200MS:                     ; 약 200 밀리초 지연을 위한 서브루틴
    LDI R18, 200           ; R18에 200을 로드 (200회 반복용 카운터)
    RCALL D200US           ; 200 마이크로초 지연 서브루틴 호출
    RCALL D200US
    RCALL D200US
    RCALL D200US
    RCALL D200US
    DEC R18                ; R18 카운터 감소
    BRNE BASE1MS           ; R18가 0이 아니라면, BASE1MS 루프로 반복
    RET                    ; 200ms 지연이 완료된 후 서브루틴 복귀

D200US:                     ; 약 200 마이크로초 지연을 위한 서브루틴
    LDI R19, 200           ; R19에 200을 로드 (200회 반복용 카운터)
BASE1US:
    NOP                    ; 1 사이클 NOP, 최소 지연 생성
    PUSH R19               ; R19 임시 저장 (스택에 푸시하여 지연 효과 추가)
    POP R19                ; R19 복원
    PUSH R19               ; 반복적으로 스택 연산을 수행하여 추가 지연 생성
    POP R19
    PUSH R19
    POP R19
    DEC R19                ; R19 카운터 감소
    BRNE BASE1US           ; R19가 0이 아니면, BASE1US 루프로 반복 (약 200마이크로초 지연)
    RET                    ; 200us 지연 후 서브루틴 복귀