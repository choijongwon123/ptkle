.include "m128def.inc"                ; ATmega128의 레지스터 및 관련 정의 포함

; 데이터 세그먼트 및 상수 정의
.DSEG
.EQU F_CPU = 16000000                 ; CPU 클럭 주파수를 16MHz로 정의

; 레지스터 별칭 정의
.DEF temp = r16                       ; r16: 임시 데이터 저장용
.DEF toggle = r17                     ; r17: 토글 플래그 (LED 전환 제어용)
.DEF RegKEYs = r18                    ; r18: PortA 관련 (Key 입력 설정값)
.DEF RegLEDs = r19                    ; r19: PortD 관련 (LED 출력 설정값)
.DEF RegCnt = r23                     ; r23: 지연 카운터
.DEF xRegL = r24                      ; r24: 10ms 지연용 루프 카운터(하위 바이트)
.DEF xRegH = r25                      ; r25: 10ms 지연용 루프 카운터(상위 바이트)

; _delay_ms 매크로 정의 (10ms 단위의 지연을 발생시킴)
.MACRO _delay_ms
    PUSH r23                         ; RegCnt(r23) 저장
    PUSH r24                         ; xRegL(r24) 저장
    PUSH r25                         ; xRegH(r25) 저장
    
    LDI RegCnt, @0 / 10              ; RegCnt에 (지연 횟수 상수)를 로드 (여기서는 10ms의 반복횟수 설정)
    RCALL DELAY_10MS                 ; 10ms 지연 서브루틴 호출 (RegCnt회 반복)
    
    POP r25                          ; xRegH(r25) 복원
    POP r24                          ; xRegL(r24) 복원
    POP r23                          ; RegCnt(r23) 복원
.ENDMACRO

; 코드 세그먼트 시작
.CSEG
.ORG 0x0000                          ; 프로그램 시작 주소 0x0000
    RJMP INIT                       ; INIT 루틴으로 점프

INIT:
    LDI R16, LOW(RAMEND)             ; RAM끝 주소(하위 바이트)를 R16에 로드
    OUT SPL, R16                     ; SPL에 R16 출력 (스택 초기화)
    LDI R16, HIGH(RAMEND)            ; RAM끝 주소(상위 바이트)를 R16에 로드
    OUT SPH, R16                     ; SPH에 R16 출력 (스택 초기화)
    
    CLR R16                          ; R16 클리어 (0으로 설정; 이후 사용되지 않음)
    
    LDI RegKEYs, 0xFD                ; RegKEYs에 0xFD 로드 (입력용 PortA 설정값; 내부 풀업 등 설정 목적)
    OUT DDRA, RegKEYs                ; DDRA에 RegKEYs 출력, PortA 핀들의 방향 설정 (입력으로)
    
    LDI RegLEDs, 0xF0                ; RegLEDs에 0xF0 로드 (출력용 PortD 설정값; LED 연결 핀 지정)
    OUT DDRD, RegLEDs                ; DDRD에 RegLEDs 출력, PortD를 출력으로 설정

LOOP:
    IN temp, PINA                    ; PortA의 값 읽어서 temp에 저장 (키 입력 상태 확인)
    ANDI temp, 0x02                  ; temp와 0x02 비트 마스크 AND 연산 (특정 키 비트 확인)
    BREQ TOGGLE_RESET                ; 만약 결과가 0이면(해당 키가 눌림) TOGGLE_RESET으로 분기
    
    CPI toggle, 0                    ; toggle과 0을 비교
    BRNE LOOP                        ; toggle이 0이 아니면(이미 토글된 상태라면) LOOP로 돌아감
    
    IN temp, PORTD                   ; 현재 PortD(LED 상태) 값을 읽어 temp에 저장
    LDI R18, 0xF0                    ; R18에 0xF0 로드 (LED 토글 마스크)
    EOR temp, R18                    ; temp와 0xF0 XOR 연산, LED 비트 토글
    OUT PORTD, temp                  ; 결과를 PortD에 출력 (LED 상태 업데이트)
    LDI toggle, 1                    ; toggle에 1 로드 (토글 상태 설정)
    
    _delay_ms(10)                    ; 10ms 지연 (매크로 호출)
    RJMP LOOP                        ; 메인 루프(LOOP)로 무조건 점프
    
TOGGLE_RESET:
    CLR toggle                       ; toggle 클리어 (0으로 설정; 토글 상태 초기화)
    RJMP LOOP                        ; 메인 루프로 점프

; 10ms 지연 서브루틴 (내부 지연 루프)
DELAY_10MS:
    ; F_CPU/4000 * 10 으로 산출된 값으로 지연 시간 결정 (10ms 지연)
    LDI xRegL, LOW(F_CPU / 4000 * 10)   ; xRegL에 지연 상수의 하위 바이트 로드
    LDI xRegH, HIGH(F_CPU / 4000 * 10)  ; xRegH에 지연 상수의 상위 바이트 로드

DELAY_LOOP:
    SBIW xRegL, 1                   ; (xRegH:xRegL) 워드에서 1 빼기 (SBIW: Subtract Immediate from Word)
    BRNE DELAY_LOOP                 ; 워드 값이 0이 아니면 계속 루프 (지연 발생)
    
    DEC RegCnt                      ; RegCnt 1 감소 (10ms 지연 반복 횟수 카운터)
    BRNE DELAY_10MS                 ; RegCnt가 0이 아니면, 10ms 지연 서브루틴을 다시 호출하여 추가 지연
    RET                             ; RegCnt가 0이면 서브루틴 종료 (전체 10ms * RegCnt 지연 완료)