#include <Adafruit_NeoPixel.h>  // WS2812B用のヘッダファイル

#define LED_PIN     D10      // WS2812Bに接続しているピン
#define NUM_LEDS    5        // 全体のLED数（5個つながっている）
#define BRIGHTNESS  10       // 明るさ（0〜255）

// ドレミファの周波数
const int tones[] = {261, 294, 329, 349, 392};    // C4, D4, E4, F4, G4　音階の周波数
const int buttons[] = {D2, D3, D4, D5, D6};       // D2～D5　タクトスイッチにつながるピン
const int speakerPin = D7;                        // D7　圧電サウンダー（スピーカー用のアンプ）につながるピン
int currentTone = -1; // 現在鳴らしている音のインデックス（-1 = 無音）

const int sw1 = D0;    // DIPスイッチ右側
const int sw2 = D1;    // DIPスイッチ左側

int rainbowCNT = 0;    // mode0 時にWS2812Bを虹色に光らせるためのカウンタ

int AmbulanceCNT = 0;   // mode1で救急車のサイレンを鳴らすときのカウンタ
int   rider[5][8] = {{100, 30, 20, 0, 0, 0, 20, 30},
                    {30, 100, 30, 20, 0, 20, 30, 100},
                    {20, 30, 100, 30, 20, 30, 100, 30},
                    {0, 20, 30, 100, 30, 100, 30, 20}, 
                    {0, 0, 20, 30, 100, 30, 20, 0} } ;      // 救急車の赤色灯を光らせるための配列
int   riderCNT = 0;     // 救急車の赤色灯を光らせるときのカウンタ

int   PoliceCNT = 0;    // パトカーのサイレンを鳴らすときのカウンタ
int   patternCNT = 0;   // パトカーのパトライトを光らせるためのカウンタ
int   bombCNT = 0;      // 爆弾を鳴らすときのカウンタ
int   popSoundCNT = 0;  // mode1のとき、popSoundを鳴らすときのカウンタ
int   popSoundCNT2 = 0; // mode1のとき、popSoundでWS2812Bを光らせるときのカウンタ
int   chordCNT = 0;     // mode2のとき、和音（？）を奏でるためのカウンタ
int   chords[5][3] = {{261, 329, 392}, // C, E, G
                      {294, 370, 440}, // D, F, A
                      {329, 415, 493}, // E, G, B
                      {349, 440, 523}, // F, A, C
                      {392, 493, 587} } ; // G, B, D（和音のコード）

int  displayCNT = 0;    // mode2のとき、WS2812Bを光らせるときのカウンタ

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);     // WS2812Bのインスタンスを取得

// 色変換関数
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

// レインボーアニメーション
void rainbowCycleStep() {

  rainbowCNT = (rainbowCNT + 2) % 255;
    for (int i = 0; i < 5; i++) {
      strip.setPixelColor(i, Wheel((i * 85 / 5 + rainbowCNT) & 255));
    }
    strip.show();
}

// ホワイト
void whiteCycle(uint8_t wait) {
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(70, 70, 70)); // 白 (R,G,B)
  }
  strip.show();
  delay(wait);
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // 白 (R,G,B)
  }
  strip.show();
  delay(wait);
}

// ダミーの関数
void colorSingle(int index) {
  ws2812Clear(); // 一度すべて消してから
  if (index >= 0 && index < NUM_LEDS) {
    strip.setPixelColor(index, strip.Color(255, 0, 0)); // 赤色
    strip.show();
  }
}

// WS2812Bをクリアする関数
void ws2812Clear(void ){
  for (int i = 0; i < 16; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // 赤 (R,G,B)
  }
  strip.show(); // 変更を反映}
}

// マシンガンの音を鳴らす関数
void playMachineGunSound(int pin) {
  tone(pin, 150);
  delay(30);
  noTone(pin);
  delay(12);
}

// 救急車のサイレンを鳴らす関数
void playAmbulanceSiren(int pin) {
  AmbulanceCNT = (AmbulanceCNT + 1) % 10;

  if(AmbulanceCNT < 5){
    tone(pin, 660);      // 低い音（ポー）
  }else if(AmbulanceCNT < 10){
    tone(pin, 880);      // 高い音（ピッ）
  }
  delay(100);
  
}

// 救急車の赤色灯を光らせる関数
void nightRider(){
  for(int i=0; i < 5; i++){
    strip.setPixelColor(i, strip.Color(rider[i][riderCNT], 0, 0)); // 赤 (R,G,B)
  }
  riderCNT = (riderCNT + 1) % 8;
  strip.show(); // 変更を反映}
}

// パトカーのサイレンを鳴らす関数
void playPoliceSiren(int pin) {
  int hertz;

  PoliceCNT = (PoliceCNT + 1) % 20;
  if(PoliceCNT < 12){
    hertz = 870 + PoliceCNT * 8;
    tone(pin, hertz);   // 高音「ファン」
    delay(80);
  }else{
    for(int i = 0; i < 6 ; i++){
      hertz = 790 - i * 12;
      tone(pin, hertz);   // 高音「ファン」
      delay(20);
    }
  }
}

// パトカーのパトライトを光らせる関数（補助）
uint32_t policeRGB(int pattern) {
  if (pattern == 0) {
    return strip.Color(150, 0, 0);
  }
  if (pattern == 1) {
    return strip.Color(0, 0, 150);
  }
}

// パトカーのパトライトを光らせる関数（メイン）
void policePattern(){
  for(int i=0; i < 5; i++){
    strip.setPixelColor(i, policeRGB(patternCNT)); // 赤 (R,G,B)
    patternCNT = (patternCNT + 1) % 2;
  }
  strip.show(); // 変更を反映}
}

// ポップなサウンドを鳴らす関数
void playCutePopSound(int pin) {
  int melody[] = { 523, 659, 784 };  // C5, E5, G5（上昇する三和音）
  int duration = 100;  // 各音の長さ（ms）

  popSoundCNT = (popSoundCNT + 1) % 3;
  tone(pin, melody[popSoundCNT]);
  delay(duration);
}

// ポップなサウンドのときにWS2812Bを光らせる関数
void popSoundRGB(){
  popSoundCNT2 = (popSoundCNT2 + 15) % 255;
  for(int i=0; i < 5; i++){
    strip.setPixelColor(i, Wheel((popSoundCNT2) & 255));
  }
  strip.show(); // 変更を反映

}

// 爆弾の音と光を発生させる関数
void playBombEffect(int pin) {
  // 落下音（低音→高音）
  strip.setBrightness(100);
  bombCNT = 0;
  for(int i = 0; i < 50; i++){
    int f = i * 18 + 300;
    tone(pin, f);
    for(int j=0; j < 5; j++){
      strip.setPixelColor(j, Wheel(((byte)(255 - i * 213 / 50)) & 255)); // 赤 (R,G,B)
    }
    strip.show(); // 変更を反映
    delay(15);
  }

  noTone(pin);
  delay(100);
  for(int i=0; i < 5; i++){
    strip.setPixelColor(i, Wheel(42));
  }
  strip.show(); // 変更を反映

  // 爆発音：ランダムな短いtone
  for(int i=0; i < 50; i++){
    tone(pin, random(100, 250));
    delay(25);
    bombCNT = bombCNT++;

    int bright = (i % 4) * 40;
    strip.setBrightness(bright);
    for(int j=0; j < 5; j++){
    strip.setPixelColor(j, Wheel(42));
    }
    strip.show(); // 変更を反映

  }
}

// 和音（コード）を奏でる関数
void playChord(int newTone) {
  int pin = D7;
  tone(pin, chords[newTone][chordCNT]);
  delay(100);  // 少し重なりつつ発音

}

// mode2のとき、WS2812Bを光らせる関数
void displayWS2812B(int newTone){
  displayCNT = (displayCNT + 1) % 2;
  if(newTone == 0){
    if(displayCNT < 1){
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(i, strip.Color(200, 0, 0)); // 赤系フェード
        strip.show();
      }
    }else if(displayCNT < 2){
      ws2812Clear();
    }
  }else if(newTone == 1){
    for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, Wheel(random(255))); // ランダム色
      strip.show();
    }
  }else if(newTone == 2){
    if(displayCNT < 1){
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(i, strip.Color(150, 150, 0));
        strip.show();
      }
    }else if(displayCNT < 2){
      ws2812Clear();
    }
  }else if(newTone == 3){
    if(displayCNT < 1){
      for (int i = 0; i < 5; i++) {
          strip.setPixelColor(i, strip.Color(0, 0, 200));
      }
      strip.show();
    }else if(displayCNT < 2){
      ws2812Clear();
    }
  }else if(newTone == 4){
    if(displayCNT < 1){
      for (int i = 0; i < 5; i++) {
        strip.setPixelColor(i, strip.Color(0, 200, 0)); // 緑系点滅
      }
      strip.show();
    }else if(displayCNT < 2){
      ws2812Clear();
    }
  }
}


// ドアのベルを鳴らす関数
void playDoorBellWithLED(int pin) {
  // ピン
  tone(pin, 1046);
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255)); // 白く光る
  }
  strip.show();
  delay(150);
  noTone(pin);
  ws2812Clear();
  delay(100);

  // ポーン
  tone(pin, 784);
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(50, 50, 255)); // 青っぽい
  }
  strip.show();
  delay(300);
  noTone(pin);
  ws2812Clear();
}

// 蒸気音を奏でる関数（失敗作）
void playSteamLocomotive(int pin) {
  // シュッシュッ（短いパルス音を繰り返し）
  for (int i = 0; i < 6; i++) {
    tone(pin, 200 + random(50)); // ノイズ風に少しばらす
    strip.setPixelColor(i % 5, strip.Color(100, 100, 100)); // 灰色に
    strip.show();
    delay(80);
    noTone(pin);
    ws2812Clear();
    delay(100);
  }

  delay(300); // 間を取る

  // ポーッ（汽笛）
  tone(pin, 440); // A4あたり
  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 255)); // 青
  }
  strip.show();
  delay(1000); // 長く鳴らす
  noTone(pin);
  ws2812Clear();
}

// 蒸気音っぽいエフェクト音（？）
void playSteamNoise(int pin) {
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 30; j++) {
      int f = random(300, 800);  // ランダム周波数でノイズ風
      tone(pin, f);
      delay(3);  // ごく短く鳴らす
    }
    noTone(pin);
    delay(100); // シュッの間隔
  }
}


// 初期設定
void setup() {
  strip.begin();            // 初期化
  strip.setBrightness(BRIGHTNESS); 

  for (int i = 0; i < 5; i++) {
    pinMode(buttons[i], INPUT_PULLDOWN); // 内部プルダウン設定（ESP32-C3で可能）
  }
  pinMode(speakerPin, OUTPUT);

  pinMode(sw1, INPUT_PULLDOWN); // 内部プルダウン設定（ESP32-C3で可能）
  pinMode(sw2, INPUT_PULLDOWN); // 内部プルダウン設定（ESP32-C3で可能）

  for (int i = 0; i < 5; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // 赤 (R,G,B)
  }
  strip.show(); // 変更を反映}


}

void loop() {
  int newTone = -1;
  int mode = 0;

  if (digitalRead(sw1) == LOW) {
    if (digitalRead(sw2) == LOW) {
      mode = 0;
    } else {
      mode = 1;
    }
  } else {
    if (digitalRead(sw2) == LOW) {
      mode = 2;
    } else {
      mode = 3;
    }
  }

  // どのボタンが押されているかチェック
  newTone = -1;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(buttons[i]) == HIGH) {
      newTone = i;
      break;
    }
  }

// mode0~3の場合分け それぞれにおいて
// どのタクトスイッチが押されたかで応答が変わる
  if (mode == 0) {
    if (newTone != -1) {
      if (newTone != currentTone) {
        tone(speakerPin, tones[newTone]);
        currentTone = newTone;
      }
      rainbowCycleStep();  // ここで毎ループ色を進める
    } else {
      noTone(speakerPin);
      ws2812Clear();
      currentTone = -1;
    }
  }else if(mode == 1){
    currentTone = newTone;
    if (newTone == 0) {
      playMachineGunSound(speakerPin);
      whiteCycle(10);
    }else if (newTone == 1) {
      nightRider();
      playAmbulanceSiren(speakerPin); // D3を押すと救急車の音
    }else if (newTone == 2) {
      policePattern();
      playPoliceSiren(speakerPin);  // D4を押すとファンファン
    }else if (newTone == 3) {
      // popSoundRGB();
      // playCutePopSound(speakerPin);

      // playDoorBellWithLED(speakerPin);

      // playSteamLocomotive(speakerPin);

      playSteamNoise(speakerPin);
    }else if (newTone == 4) {
      playBombEffect(speakerPin);
    }
    noTone(speakerPin);
    ws2812Clear();
  }else if(mode == 2){
    if (newTone != -1) {
      currentTone = newTone;
      chordCNT = (chordCNT + 1) % 3;
      displayWS2812B(newTone);
      playChord(newTone);
    } else {
      noTone(speakerPin);
      ws2812Clear();
      currentTone = -1;
    }
  }else if(mode == 3){
    // 今後の拡張モード用
    ws2812Clear();
    noTone(speakerPin);
  }


  delay(10);

}
