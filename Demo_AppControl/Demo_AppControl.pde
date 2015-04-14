import processing.serial.*;

// Arduinoとの通信用インスタンス
Serial port;                // シリアル通信用のポート
int infoType = 0;           // シリアルに書き込まれている情報の種類
int infoNum = 2;            // 書き込まれる情報の種類の数 
int switchType = 0;         // スイッチの種類
int touchFlg = 0;           // スイッチに触れてるかどうか

// アプリケーション用の素材インスタンス
PImage[] animal;    // 動物イラスト
PImage backImage;   // 背景画像
int cell = 8;       // 画像の数
int number = 0;


void setup(){
    // シリアルポートの設定
    println(Serial.list());                // シリアルポート一覧
    String portName = Serial.list()[5];    // Arduinoと接続しているシリアルを選択
    port = new Serial(this, portName, 9600);
    
    println("Byte in Port");
    println(port.available());
    // 画面設定
    size(640, 640);
    
    // 素材ロード
    // 動物イラスト
    animal = new PImage[cell];
    for(int i = 0; i < cell; i++){
        animal[i] = loadImage("animal" + (i + 1) + ".png");
    }
    // 背景
    backImage = loadImage("background.jpg");
}

void draw(){
    // 背景画像の描画
    background(backImage);
    
    // if switch exist
    if(switchType > 0){
        switchApp();
    }
    
}

/* switch Application */
void switchApp(){
    switch(switchType){
        case 1:
            number =  switchType + (touchFlg * 4);
            break;
        case 2:
            number =  switchType + (touchFlg * 4);
            break;
        case 3:
            number =  switchType + (touchFlg * 4);
            break;
        case 4:
            number = switchType + (touchFlg * 4);
            break;  
        default:
            break;
    }
    number--;
    // output animal image
    image(animal[number], width/2-(width/3)/2, height/2-(height/3)/2+150, width/3, height/3);
    
}

/* シリアル通信 */
void serialEvent(Serial p){
    /* 改行区切りでデータを読み込む (¥n == 10) */
    String inString = p.readStringUntil(10);

    /* ２つ以上のデータが存在している場合、数値として読み込む */
    if(inString != null){
      inString = trim(inString);
      int[] value = int(split(inString, ','));
      
      if(value.length > 1){
         switchType = value[0];
         touchFlg = value[1];
      }
    }
    
    println("スイッチの種類：" + switchType);  
    println("触れてるかどうか：" + touchFlg);
}

