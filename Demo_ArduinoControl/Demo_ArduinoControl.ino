#include <CapacitiveSensor.h>

// 静電容量・センサ
const int transPin = 2;    // センサとして使用する送信側のピン番号
const int recievePin = 3;  // センサとして使用する受信側のピン番号
const int sensorPin = 5;   // 電圧を読み取るピン(AnalogInput)
CapacitiveSensor CapSensor = CapacitiveSensor(transPin, recievePin);  // 静電容量センサ

// バッファ
const int BUFFER_LENGTH = 5;                    // バッファのサイズ
const int INDEX_OF_MIDDLE = BUFFER_LENGTH / 2;  // バッファの中央のインデックス
long volt_buffer1[BUFFER_LENGTH];                // 電圧バッファ1
long volt_buffer2[BUFFER_LENGTH];                // 電圧バッファ2
long cap_buffer1[BUFFER_LENGTH];                // 静電容量バッファ1
long cap_buffer2[BUFFER_LENGTH];                // 静電容量バッファ2
int firstCap = 0;                               // 静電容量セットアップ認識用
int index = 0;                                  // 電圧バッファに書き込むインデックス
int capIndex = 0;                               // 静電容量バッファに書き込むインデックス
int LoopCount = BUFFER_LENGTH * BUFFER_LENGTH;  // 平均値を用いるために、セットアップ時に LoopCount 分 loop() を回しておく

// スイッチ
int switchNum = 4;            // スイッチの種類の数
int switchRange[4][2];        // 各スイッチの電圧範囲（[switchNum][最小値, 最大値]）
int switchType = 0;           // スイッチの種類（0 = 存在しない）
const int thresholdH = 1000;    // しきい値 High
const int thresholdL = 100;    // しきい値 Low
boolean preTouched = false;   // 前回タッチしていたかどうか

int wasTmp = 0;
int isTmp = 0;

void setup() {
    CapSensor.reset_CS_AutoCal();   // 静電容量センサをキャリブレーション
    Serial.begin(9600);             // シリアル通信を開始
    initSwitch();                   // 各スイッチの電圧範囲の初期化
    for(int i = 0; i < LoopCount; i++){
        SenseVolt();
    }
}

void loop() {
    /* タッチ状態を引き継ぐ */
    boolean curTouched = preTouched;
    
    /* 電圧を測定（2段階スムージング） */                                
    long Volt = SenseVolt();
    Serial.print("Volt = ");
    Serial.println(Volt);
    
//    Serial.println(switchType);
    
    /* スイッチが存在するとき */
    if(switchExist(Volt)){
        /* 静電容量を測定 */
        // 静電容量のセットアップ（事前スムージング処理）
        if(firstCap == 0){
            for(int i = 0; i < LoopCount; i++){
                SenseCap();
            }
            firstCap++;
        }
        long Cap = SenseCap();
        Serial.print("Cap = ");
        Serial.println(Cap);
        
        /* タッチ判定 */
        // 測定した静電容量の値が上のしきい値より大きければ触れていると判定
        if(Cap > thresholdH){
            curTouched = true;
//            isTmp = 1;
//            if(isTmp != wasTmp){
//                Serial.println("ON");
//                Serial.println(Cap);
//                wasTmp = 1;
//            }
        }
        // 測定した静電容量の値が下のしきい値より小さければ触れていないと判定
        else if(Cap < thresholdL){
            curTouched = false;
//            isTmp = 0;
//            if(isTmp != wasTmp){
//                Serial.println("OFF");
//                Serial.println(Cap);
//                wasTmp = 0;
//            }
        }
        // ※しきい値の上と下の間である場合は、前回のタッチ状態を引き継ぐ
        preTouched = curTouched;    // 現在のタッチ状態を保存しておく
    }
    
    /* スイッチの種類と触れているかどうかをシリアルに書き込む */
    /* 複数の数値を通信したいときは、一つの文字列にまとめて、processing 側でパースする必要がある */
//    Serial.print(switchType);
//    Serial.print(",");
//    Serial.println(curTouched);
    delay(100);
}

/* 電圧を測定 */
long SenseVolt(){
    long raw = analogRead(sensorPin);                         // AnalogInputから電圧値を測定
    volt_buffer1[index] = raw;                               // その結果を buffer1 に蓄積する
    long average = smoothByMeanFilter(volt_buffer1);     // Meanフィルタでスムージング
    volt_buffer2[index] = average;                           // 電圧の平均値を蓄積
    long FilterVoltValue = smoothByMeanFilter(volt_buffer2);    // スムージングした電圧値を保存
    index = (index + 1) % BUFFER_LENGTH;                     // インデックス更新
    
    return FilterVoltValue;
}

/* 静電容量を測定 */
long SenseCap(){
    long raw = CapSensor.capacitiveSensor(10);
    cap_buffer1[capIndex] = raw;
    long average = smoothByMeanFilter(cap_buffer1);
    cap_buffer2[capIndex] = average;
    long FilterCapValue = smoothByMeanFilter(cap_buffer2);
    capIndex = (capIndex + 1) % BUFFER_LENGTH;             // インデックス更新
    
    return FilterCapValue;
}

/* スイッチが取り付けられているかどうか判定する */
boolean switchExist(int val){
    for(int i = 0; i < switchNum; i++){
        if(switchRange[i][0] < val && val < switchRange[i][1]){
            switchType = i + 1;    // スイッチの種類を保存しておく
            //Serial.println(switchType);
            return true;           
        }
    }
    // スイッチが取り付けられていないとき
    switchType = 0;
    return false; 
}

/* Meanフィルタ(平均化)によるスムージング */
long smoothByMeanFilter(long *box){
    long sum = 0;
    // 電圧値の合計を求める
    for(int i = 0; i < BUFFER_LENGTH; i++){
        sum += box[i];
    }
    // その平均を返す
    return (long)(sum / BUFFER_LENGTH);
}

/* 各スイッチの電圧範囲の初期化 */
void initSwitch(){
    // スイッチ 2
    switchRange[1][0] = 800;    // min
    switchRange[1][1] = 850;    // max
    // スイッチ 4
    switchRange[3][0] = 680;    // min
    switchRange[3][1] = 735;    // max
    // スイッチ 1
    switchRange[0][0] = 465;    // min
    switchRange[0][1] = 499;    // max
    // スイッチ 3
    switchRange[2][0] = 180;    // min = 250
    switchRange[2][1] = 210;    // max = 300
}
