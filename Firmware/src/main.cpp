#include <Arduino.h>
#include <SEGGER_RTT.h>

// =========================== CONFIG ===========================
#define MAX_TARGET_EMF 30000  // максимальний BEMF [мВ] 
#define DIV_R1 20000          // верхнє плече дільника [Ом]
#define DIV_R2 2200           // нижнє плече дільника [Ом]
#define PID_DIVIDER 16000.0   

#define MIN_TARGET_EMF 50     // ЗНИЖЕНО: щоб мотор стартував від легкого повороту ручки
#define MIN_VMOT 6000         // мінімальне живлення [мВ] (6В)
#define CTRL_PRD 20           // період управління [мс]
#define WAIT_ADC 800          // пауза після вимкнення ШИМ [мкс]
#define SMOOTH_STEP 100       // крок плавного пуску

#define FIXED_P_RAW 1500      // Фіксований Kp (оскільки ручки зайняті Speed, I, D)

// Піни МК
#define BEMF_PIN  PA0   
#define VMOT_PIN  PA1   
#define SPEED_PIN PA2   // A2 - Скорість (Target)
#define I_PIN     PA3   // A3 - Integral (Ki)
#define D_PIN     PA4   // A4 - Derivative (Kd)
#define PWM_PIN   PB0   

// =========================== DATA ===========================
const int vref = 3300;   // 3.3V
int target = 0;          
int pwm = 0;             

// ========================== IntEMA ==========================
template <typename T>
class IntEMA {
   public:
    T filter(T val, uint8_t k2) {
        T sum = (val - _filt) + _err;
        T div = sum >> k2;
        _err = sum - (div << k2);
        return _filt += div;
    }
    void init(T val) {
        _filt = val;
        _err = 0;
    }
    T get() { return _filt; }
   private:
    T _filt = 0, _err = 0;
};

IntEMA<int> vmot;
IntEMA<int> vemf;
IntEMA<int> a_tar;
IntEMA<int> a_i;
IntEMA<int> a_d;

// =========================== PIDreg ===========================
class PIDreg {
   public:
    float Kp = 0, Ki = 0, Kd = 0;
    float integral = 0;
    float lastError = 0;
    bool firstRun = true;

    int compute(int input, int setpoint, int feedforward, float dt) {
        float error = setpoint - input;
        
        // Інтегральна складова
        float nextIntegral = integral + error * Ki * dt;

        // Диференціальна складова (по похибці)
        float derivative = 0;
        if (!firstRun && dt > 0) {
            derivative = (error - lastError) / dt;
        } else {
            firstRun = false;
        }
        lastError = error;

        // Підсумковий вихід ПІД + FeedForward
        float output = feedforward + (error * Kp) + nextIntegral + (derivative * Kd);

        // Anti-windup (захист інтеграла від переповнення при насиченні PWM)
        if (!((output > 255 && error > 0) || (output < 0 && error < 0))) {
            integral = nextIntegral;
        }

        return constrain((int)output, 0, 255);
    }

    void reset() {
        integral = 0;
        lastError = 0;
        firstRun = true;
    }
};
PIDreg pid;

// =========================== FUNC ===========================

void setPWM(uint8_t duty) {
    analogWrite(PWM_PIN, duty);
}

uint16_t readDivider(uint8_t apin) {
    uint32_t pinMv = (uint32_t)analogRead(apin) * vref / 4096;
    return (pinMv * (DIV_R1 + DIV_R2)) / DIV_R2;
}

void control() {
    static uint32_t tmr;
    if (millis() - tmr < CTRL_PRD) return;
    float dt = (millis() - tmr) / 1000.0f;
    tmr = millis();

    // 1. Вимикаємо ШИМ для заміру BEMF
    setPWM(0);
    delayMicroseconds(WAIT_ADC);

    // 2. Читаємо напруги
    int rawVmot = readDivider(VMOT_PIN);
    int rawMotor = readDivider(BEMF_PIN);
    
    int rawBemf = rawVmot - rawMotor;
    if (rawBemf < 0) rawBemf = 0; 

    // Відновлюємо ШИМ 
    if (rawVmot > MIN_VMOT) setPWM(pwm);

    // 3. Фільтруємо вхідні дані
    vmot.filter(rawVmot, 2);
    vemf.filter(rawBemf, 2);
    a_tar.filter(analogRead(SPEED_PIN), 3);
    a_i.filter(analogRead(I_PIN), 3);
    a_d.filter(analogRead(D_PIN), 3);

    int newTarget = (long)a_tar.get() * MAX_TARGET_EMF / 4096;

    // Розрахунок коефіцієнтів з потенціометрів
    pid.Kp = (float)FIXED_P_RAW / PID_DIVIDER;
    pid.Ki = (float)a_i.get() / PID_DIVIDER;
    pid.Kd = (float)a_d.get() / PID_DIVIDER;

    // 4. ПЕРЕВІРКА ЗАХИСТІВ
    if (rawVmot <= MIN_VMOT) {
        SEGGER_RTT_printf(0, "[BLOCKED] VMOT too low: %d mV\r\n", rawVmot);
        pid.reset(); target = 0; pwm = 0; setPWM(0);
        return;
    }
    
    if (newTarget < MIN_TARGET_EMF) {
        SEGGER_RTT_printf(0, "[BLOCKED] Target too low: %d mV (Pot: %d)\r\n", newTarget, a_tar.get());
        pid.reset(); target = 0; pwm = 0; setPWM(0);
        return;
    }

    // 5. Плавний пуск та розрахунок
    target += constrain(newTarget - target, -SMOOTH_STEP, SMOOTH_STEP);

    int current_vmot = vmot.get();
    if (current_vmot < 1000) current_vmot = 1000; 

    int feedforward = (long)target * 255 / current_vmot;

    pwm = pid.compute(vemf.get(), target, feedforward, dt);
    setPWM(pwm);

    // 6. УСПІШНИЙ ЛОГ РОБОТИ
    SEGGER_RTT_printf(0, "RUN | Tar:%d mV | BEMF:%d mV | FF:%d | PWM:%d/255 | Ki: %d/1000 | Kd: %d/1000\r\n", 
                      target, vemf.get(), feedforward, pwm, (int)(pid.Ki * 1000), (int)(pid.Kd * 1000));
}

void setup() {
    SEGGER_RTT_Init();
    SEGGER_RTT_WriteString(0, "\r\n=== BEMF PID Controller Started ===\r\n");

    analogReadResolution(12);
    pinMode(PWM_PIN, OUTPUT);
    analogWriteResolution(8); 
    analogWriteFrequency(16000); 

    int startVmot = readDivider(VMOT_PIN);
    vmot.init(startVmot > 0 ? startVmot : 12000);
    vemf.init(0);
}

void loop() {
    control();
}